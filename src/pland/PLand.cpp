#include "pland/PLand.h"
#include "BuildInfo.h"

#include <memory>

#include "ll/api/data/Version.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/io/LogLevel.h"
#include "ll/api/mod/RegisterHelper.h"
#include "ll/api/thread/ThreadPoolExecutor.h"
#include "ll/api/utils/SystemUtils.h"

#include "drawer/DrawHandleManager.h"
#include "events/domain/ConfigReloadEvent.h"
#include "internal/interceptor/InterceptorConfig.h"
#include "land/internal/LandScheduler.h"
#include "land/internal/SafeTeleport.h"
#include "pland/economy/EconomySystem.h"
#include "pland/internal/adapter/telemetry/Telemetry.h"
#include "pland/internal/command/Command.h"
#include "pland/internal/interceptor/EventInterceptor.h"
#include "pland/land/Config.h"
#include "pland/land/repo/LandRegistry.h"
#include "pland/selector/SelectorManager.h"
#include "pland/service/ServiceLocator.h"


#ifdef LD_DEVTOOL
#include "DevToolApp.h"
#endif

#ifdef PLAND_SCRIPTING
#include "pland/scripting/Bootstrap.h"
#endif

namespace land {

namespace {

/// 作用域退出时执行回调, dismiss() 之后不再执行
template <class F>
class ScopeGuard {
    F    mCallback;
    bool mActive = true;

public:
    explicit ScopeGuard(F callback) : mCallback(std::move(callback)) {}
    ~ScopeGuard() {
        if (mActive) {
            mCallback();
        }
    }

    void dismiss() { mActive = false; }
};

} // namespace

struct PLand::Impl {
    ll::mod::NativeMod& mSelf;

    /// 线程池。只由 destroyThreadPool() 显式销毁, 不随 Impl 析构自动销毁:
    /// 析构链在 DLL 卸载路径 (FreeLibrary → DllMain → atexit) 上执行, 该路径持有 loader lock,
    /// 而 join 工作线程要等线程退出, 线程退出 (LdrShutdownThread) 又要 loader lock, 二者互锁。
    ll::thread::ThreadPoolExecutor*                          mThreadPoolExecutor{nullptr};
    std::unique_ptr<LandRegistry>                            mLandRegistry{nullptr};
    std::unique_ptr<internal::LandScheduler>                 mLandScheduler{nullptr};
    std::unique_ptr<internal::interceptor::EventInterceptor> mEventListener{nullptr};
    std::unique_ptr<internal::SafeTeleport>                  mSafeTeleport{nullptr};
    std::unique_ptr<SelectorManager>                         mSelectorManager{nullptr};
    std::unique_ptr<DrawHandleManager>                       mDrawHandleManager{nullptr};
    std::unique_ptr<internal::adapter::Telemetry>            mTelemetry{nullptr};

    ll::event::ListenerPtr mConfigReloadListener{nullptr};

    std::unique_ptr<service::ServiceLocator> mServiceLocator{nullptr};

#ifdef LD_DEVTOOL
    std::unique_ptr<devtool::DevToolApp> mDevToolApp{nullptr};
#endif

#ifdef PLAND_SCRIPTING
    std::unique_ptr<scripting::Bootstrap> mBootstrap{nullptr};
#endif

    explicit Impl() : mSelf(*ll::mod::NativeMod::current()) {}

    /// 销毁线程池并 join 工作线程, 幂等。
    /// 只能在 DLL 卸载之前调用 (disable / load 失败清理), 否则会撞上 loader lock 死锁。
    void destroyThreadPool() {
        if (!mThreadPoolExecutor) {
            return;
        }
        mThreadPoolExecutor->destroy(); // join 工作线程
        delete mThreadPoolExecutor;     // impl 已由 destroy() 释放, 这里只释放外壳
        mThreadPoolExecutor = nullptr;
    }
};

bool ensureStableVersion() {
    auto tag = land::BuildInfo::kBuildTag;
    if (tag.find("-g") != std::string_view::npos) {
        return false;
    }
    if (!tag.empty() && tag.front() == 'v') {
        tag.remove_prefix(1);
    }
    return ll::data::Version::valid(tag);
}

bool PLand::load() {
    auto& logger = getSelf().getLogger();
    logger.info("PLand - Open-source project based on AGPL v3 license");
    logger.info("Repository: https://github.com/IceBlcokMC/PLand");
    logger.info("Issues:     https://github.com/IceBlcokMC/PLand/issues");
    logger.info("Copyright (C) 2024-present IceBlcokMC Team and contributors");

    logger.info(
        "Build: {} (Branch: {}, Commit: {}, Variant: {})",
        BuildInfo::kBuildTag,
        BuildInfo::kBuildBranch,
        BuildInfo::kBuildCommit,
        BuildInfo::kBuildVariant
    );

    if (!ensureStableVersion()) {
        logger.warn("This is a development build ({}). It may not be stable.", BuildInfo::kBuildTag);
    }

    logger.info("Loading PLand...");
    if (auto res = ll::i18n::getInstance().load(getSelf().getLangDir()); !res) {
        logger.error("Load language file failed, plugin will use default language.");
        res.error().log(logger);
    }

    internal::interceptor::InterceptorConfig::tryMigrateLegacyConfig(getSelf().getConfigDir());

    if (!loadConfig()) {
        logger.error("Failed to load config"); // loadConfig internal log the error message
        return false;
    }

    if (auto ok = internal::interceptor::InterceptorConfig::load(getSelf().getConfigDir()); !ok) {
        logger.error("Failed to load interceptor config, reason: {}", ok.error().message());
        return false;
    }

    // 从这里开始建立运行期资源, 失败路径 (提前 return / 异常) 都要在 load 内就地拆除:
    // 留给静态析构会落在 DLL 卸载路径的 loader lock 内, 那里 join 线程池会死锁。
    auto rollback = ScopeGuard{[this] {
#ifdef PLAND_SCRIPTING
        mImpl->mBootstrap.reset();
#endif
        mImpl->mLandRegistry.reset(); // 停掉落盘协程: stop() 以 interrupt(true) 在调用线程就地 resume
        mImpl->destroyThreadPool();
    }};

    mImpl->mThreadPoolExecutor = new ll::thread::ThreadPoolExecutor{"PLand-ThreadPool", 4};

    mImpl->mLandRegistry = std::make_unique<land::LandRegistry>(*this);

    EconomySystem::getInstance().initialize();

#ifdef PLAND_SCRIPTING
    logger.info("Initializing script engine...");
    mImpl->mBootstrap = std::make_unique<scripting::Bootstrap>(getSelf());
    if (auto e = mImpl->mBootstrap->initialize(); !e) {
        e.error().log(logger);
        return false; // rollback 就地拆除
    }
    logger.info("Initializing script engine...done");
    if (auto e = mImpl->mBootstrap->postOnLoad(); !e) {
        e.error().log(logger);
    }
#endif

#ifdef PLAND_DEBUG
    logger.warn("Debug Mode");
    logger.setLevel(ll::io::LogLevel::Trace);
#endif

    rollback.dismiss();
    return true;
}

bool PLand::enable() {
    internal::LandCommand::setupAll();
    mImpl->mLandScheduler     = std::make_unique<internal::LandScheduler>();
    mImpl->mEventListener     = std::make_unique<internal::interceptor::EventInterceptor>();
    mImpl->mSafeTeleport      = std::make_unique<internal::SafeTeleport>();
    mImpl->mSelectorManager   = std::make_unique<SelectorManager>();
    mImpl->mDrawHandleManager = std::make_unique<DrawHandleManager>();
    mImpl->mTelemetry         = std::make_unique<internal::adapter::Telemetry>();
    if (ConfigProvider::isTelemetryEnabled()) {
        mImpl->mTelemetry->launch(getThreadPool());
    }

    mImpl->mServiceLocator = std::make_unique<service::ServiceLocator>(*this);

    mImpl->mConfigReloadListener = ll::event::EventBus::getInstance().emplaceListener<event::ConfigReloadEvent>(
        [this](event::ConfigReloadEvent& ev [[maybe_unused]]) {
            (void)internal::interceptor::InterceptorConfig::load(getSelf().getConfigDir());

            mImpl->mEventListener->reload();

            EconomySystem::getInstance().reload();

            if (ConfigProvider::isTelemetryEnabled()) {
                mImpl->mTelemetry->launch(getThreadPool());
            } else {
                mImpl->mTelemetry->shutdown();
            }

            mImpl->mDrawHandleManager.reset();
            mImpl->mDrawHandleManager = std::make_unique<DrawHandleManager>();
        }
    );

#ifdef LD_DEVTOOL
    if (ConfigProvider::isDevToolsEnabled()) {
        mImpl->mDevToolApp = devtool::DevToolApp::make();
    }
#endif
#ifdef PLAND_SCRIPTING
    if (auto e = mImpl->mBootstrap->postOnEnable(); !e) {
        e.error().log(getSelf().getLogger());
    }
#endif

    return true;
}

bool PLand::disable() {
    auto& logger = mImpl->mSelf.getLogger();

#ifdef LD_DEVTOOL
    if (ConfigProvider::isDevToolsEnabled()) {
        mImpl->mDevToolApp.reset();
    }
#endif
#ifdef PLAND_SCRIPTING
    if (mImpl->mBootstrap) { // load 失败时已由 rollback 拆除
        if (auto e = mImpl->mBootstrap->postOnDisable(); !e) {
            e.error().log(logger);
        }
        if (auto e = mImpl->mBootstrap->shutdown(); !e) {
            e.error().log(logger);
        }
        mImpl->mBootstrap.reset();
    }
#endif

    ll::event::EventBus::getInstance().removeListener(mImpl->mConfigReloadListener);

    mImpl->mTelemetry.reset();

    mImpl->mServiceLocator.reset();

    logger.debug("Destroying resources...");
    mImpl->mLandScheduler.reset();
    mImpl->mEventListener.reset();
    mImpl->mSafeTeleport.reset();
    mImpl->mSelectorManager.reset();
    mImpl->mDrawHandleManager.reset();
    mImpl->mLandRegistry.reset();

    logger.debug("Destroying thread pool...");
    mImpl->destroyThreadPool();
    return true;
}

bool PLand::unload() { return true; }

PLand& PLand::getInstance() {
    static PLand instance;
    return instance;
}

PLand::PLand() : mImpl(std::make_unique<Impl>()) {}

ll::mod::NativeMod&     PLand::getSelf() const { return mImpl->mSelf; }
SelectorManager*        PLand::getSelectorManager() const { return mImpl->mSelectorManager.get(); }
LandRegistry&           PLand::getLandRegistry() const { return *mImpl->mLandRegistry; }
DrawHandleManager*      PLand::getDrawHandleManager() const { return mImpl->mDrawHandleManager.get(); }
internal::SafeTeleport& PLand::getSafeTeleport() const { return *mImpl->mSafeTeleport; }
bool                    PLand::loadConfig() {
    if (auto expected = ConfigProvider::load(getSelf().getConfigDir())) {
        return true;
    } else {
        auto& logger = mImpl->mSelf.getLogger();
        expected.error().log(logger);
        return false;
    }
}
bool PLand::saveConfig() {
    if (auto expected = ConfigProvider::save(getSelf().getConfigDir())) {
        return true;
    } else {
        auto& logger = mImpl->mSelf.getLogger();
        expected.error().log(logger);
        return false;
    }
}

ll::thread::ThreadPoolExecutor& PLand::getThreadPool() const { return *mImpl->mThreadPoolExecutor; }
service::ServiceLocator&        PLand::getServiceLocator() const { return *mImpl->mServiceLocator; }

#ifdef LD_DEVTOOL
void PLand::setDevToolVisible(bool visible) {
    if (ConfigProvider::isDevToolsEnabled()) {
        if (visible) {
            mImpl->mDevToolApp->show();
        } else {
            mImpl->mDevToolApp->hide();
        }
    }
}
#endif

#ifdef PLAND_SCRIPTING
scripting::Bootstrap& PLand::getScriptingBootstrap() { return *mImpl->mBootstrap; }
void                  PLand::reloadScripting() {
    auto& logger = getSelf().getLogger();
    logger.info("Reloading script, please wait...");

    auto& boot = getScriptingBootstrap();
    if (auto ex = boot.reload(); !ex) {
        logger.error("Failed to reload script.");
        ex.error().log(logger);
        return;
    }

    if (auto e = mImpl->mBootstrap->postOnLoad(); !e) {
        e.error().log(logger);
    }
    if (auto e = mImpl->mBootstrap->postOnEnable(); !e) {
        e.error().log(logger);
    }

    logger.info("done!");
}
#endif


} // namespace land

LL_REGISTER_MOD(land::PLand, land::PLand::getInstance());
