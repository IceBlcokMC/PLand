#include "pland/PLand.h"

#include <memory>

#include "ll/api/Versions.h"
#include "ll/api/data/Version.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/io/LogLevel.h"
#include "ll/api/mod/RegisterHelper.h"
#include "ll/api/thread/ThreadPoolExecutor.h"
#include "ll/api/utils/SystemUtils.h"

#include "pland/Version.h"
#include "pland/command/Command.h"
#include "pland/economy/EconomySystem.h"
#include "pland/events/ConfigReloadEvent.h"
#include "pland/hooks/EventListener.h"
#include "pland/infra/Config.h"
#include "pland/infra/DrawHandleManager.h"
#include "pland/infra/SafeTeleport.h"
#include "pland/land/LandRegistry.h"
#include "pland/land/LandScheduler.h"
#include "pland/network/telemetry/Telemetry.h"
#include "pland/selector/SelectorManager.h"


#ifdef LD_TEST
#include "TestMain.h"
#endif

#ifdef LD_DEVTOOL
#include "DevToolApp.h"
#endif

namespace land {


struct PLand::Impl {
    ll::mod::NativeMod&                             mSelf;
    std::unique_ptr<ll::thread::ThreadPoolExecutor> mThreadPoolExecutor{nullptr};
    std::unique_ptr<LandRegistry>                   mLandRegistry{nullptr};
    std::unique_ptr<EventListener>                  mEventListener{nullptr};
    std::unique_ptr<LandScheduler>                  mLandScheduler{nullptr};
    std::unique_ptr<SafeTeleport>                   mSafeTeleport{nullptr};
    std::unique_ptr<SelectorManager>                mSelectorManager{nullptr};
    std::unique_ptr<DrawHandleManager>              mDrawHandleManager{nullptr};
    ll::event::ListenerPtr                          mConfigReloadListener{nullptr};
    std::unique_ptr<network::Telemetry>             mTelemetry{nullptr};

#ifdef LD_DEVTOOL
    std::unique_ptr<devtool::DevToolApp> mDevToolApp{nullptr};
#endif

public: // API
    explicit Impl() : mSelf(*ll::mod::NativeMod::current()) {
        // !! 这里的构造时机很早，请不要在这里初始化任何带有依赖的资源 !!
    }

    ~Impl() {
        // !! 务必注意析构顺序 !!
#ifdef LD_DEVTOOL
        if (land::Config::cfg.internal.devTools) {
            this->mDevToolApp.reset();
        }
#endif
        ll::event::EventBus::getInstance().removeListener(this->mConfigReloadListener);

        auto& logger = mSelf.getLogger();
        this->mTelemetry.reset();

        logger.debug("Saving land registry...");
        this->mLandRegistry->save();

        logger.debug("Destroying resources...");
        this->mLandScheduler.reset();
        this->mEventListener.reset();
        this->mSafeTeleport.reset();
        this->mSelectorManager.reset();
        this->mDrawHandleManager.reset();
        this->mLandRegistry.reset();

        logger.debug("Destroying thread pool...");
        this->mThreadPoolExecutor->destroy();
        this->mThreadPoolExecutor.reset();
    }
};


bool PLand::load() {
    auto& logger = getSelf().getLogger();
    logger.info(R"(  _____   _                        _ )");
    logger.info(R"( |  __ \ | |                      | |)");
    logger.info(R"( | |__) || |      __ _  _ __    __| |)");
    logger.info(R"( |  ___/ | |     / _` || '_ \  / _` |)");
    logger.info(R"( | |     | |____| (_| || | | || (_| |)");
    logger.info(R"( |_|     |______|\__,_||_| |_| \__,_|)");
    logger.info(R"(                                     )");
    logger.info("Loading...");

    if (PLAND_VERSION_SNAPSHOT) {
        logger.warn("Version: {}", PLAND_VERSION_STRING);
        logger.warn("您当前正在使用开发快照版本，此版本可能某些功能异常、损坏、甚至导致崩溃，请勿在生产环境中使用。");
        logger.warn(
            "You are using a development snapshot version, this version may have some abnormal, broken or even "
            "crash functions, please do not use it in production environment."
        );
    } else {
        logger.info("Version: {}", PLAND_VERSION_STRING);
    }

#ifdef LEVI_LAMINA_VERSION
    logger.info("LeviLamina Version: {}", LEVI_LAMINA_VERSION);
    auto const  semver    = ll::data::Version{LEVI_LAMINA_VERSION};
    const auto& llVersion = ll::getLoaderVersion();
    // 仅检查 major 和 minor 版本号
    if (llVersion.major != semver.major || llVersion.minor != semver.minor) {
        logger.warn(
            "插件所依赖的 LeviLamina 版本 ({}) 与当前运行的版本 ({}) 不匹配。",
            LEVI_LAMINA_VERSION,
            llVersion.to_string()
        );
        logger.warn("这可能会让插件无法正常工作!建议使用与插件依赖版本相同的 LeviLamina 版本。");
        logger.warn("这可能导致数据丢失或服务器不稳定。");
        logger.warn("请谨慎使用，并随时准备好备份。");
        logger.warn(
            "The LeviLamina version ({}) that the plugin depends on does not match the currently running version ({}).",
            LEVI_LAMINA_VERSION,
            llVersion.to_string()
        );
        logger.warn("This may cause data loss or server instability.");
        logger.warn("Please use with caution and be prepared to back up at any time.");
    }
#else
    logger.info("LeviLamina Version: Unknown");
#endif

#ifdef ILISTENATTENTIVELY_VERSION
    logger.info("iListenAttentively Version: {}", ILISTENATTENTIVELY_VERSION);
#else
    logger.info("iListenAttentively Version: Unknown");
#endif

    if (auto res = ll::i18n::getInstance().load(getSelf().getLangDir()); !res) {
        logger.error("Load language file failed, plugin will use default language.");
        res.error().log(logger);
    }

    land::Config::tryLoad();
    logger.setLevel(land::Config::cfg.logLevel);

    mImpl->mThreadPoolExecutor = std::make_unique<ll::thread::ThreadPoolExecutor>("PLand-ThreadPool", 2);

    mImpl->mLandRegistry = std::make_unique<land::LandRegistry>();
    land::EconomySystem::getInstance().initEconomySystem();

#ifdef DEBUG
    logger.warn("Debug Mode");
    logger.setLevel(ll::io::LogLevel::Trace);
#endif

    return true;
}

bool PLand::enable() {
    land::LandCommand::setup();
    mImpl->mLandScheduler     = std::make_unique<land::LandScheduler>();
    mImpl->mEventListener     = std::make_unique<land::EventListener>();
    mImpl->mSafeTeleport      = std::make_unique<land::SafeTeleport>();
    mImpl->mSelectorManager   = std::make_unique<land::SelectorManager>();
    mImpl->mDrawHandleManager = std::make_unique<land::DrawHandleManager>();
    mImpl->mTelemetry         = std::make_unique<network::Telemetry>(this);

    mImpl->mConfigReloadListener = ll::event::EventBus::getInstance().emplaceListener<events::ConfigReloadEvent>(
        [this](events::ConfigReloadEvent& ev [[maybe_unused]]) {
            mImpl->mEventListener.reset();
            mImpl->mEventListener = std::make_unique<land::EventListener>();

            EconomySystem::getInstance().reloadEconomySystem();

            mImpl->mTelemetry.reset();
            mImpl->mTelemetry = std::make_unique<network::Telemetry>(this);
        }
    );


#ifdef LD_TEST
    test::TestMain::setup();
#endif

#ifdef LD_DEVTOOL
    if (land::Config::cfg.internal.devTools) {
        mImpl->mDevToolApp = devtool::DevToolApp::make();
    }
#endif

    return true;
}

bool PLand::disable() {
    mImpl.reset();
    return true;
}

bool PLand::unload() { return true; }

PLand& PLand::getInstance() {
    static PLand instance;
    return instance;
}

PLand::PLand() : mImpl(std::make_unique<Impl>()) {}

ll::mod::NativeMod& PLand::getSelf() const { return mImpl->mSelf; }
SafeTeleport*       PLand::getSafeTeleport() const { return mImpl->mSafeTeleport.get(); }
LandScheduler*      PLand::getLandScheduler() const { return mImpl->mLandScheduler.get(); }
SelectorManager*    PLand::getSelectorManager() const { return mImpl->mSelectorManager.get(); }
LandRegistry*       PLand::getLandRegistry() const { return mImpl->mLandRegistry.get(); }
DrawHandleManager*  PLand::getDrawHandleManager() const { return mImpl->mDrawHandleManager.get(); }

ll::thread::ThreadPoolExecutor* PLand::getThreadPool() const { return mImpl->mThreadPoolExecutor.get(); }

#ifdef LD_DEVTOOL
devtool::DevToolApp* PLand::getDevToolApp() const { return mImpl->mDevToolApp.get(); }
#endif


} // namespace land

LL_REGISTER_MOD(land::PLand, land::PLand::getInstance());
