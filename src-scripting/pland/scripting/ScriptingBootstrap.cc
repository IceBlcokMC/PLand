#include "ScriptingBootstrap.h"

#include "pland/scripting/EngineHelper.h"
#include "pland/scripting/community/CommunityPluginLoader.h"
#include "pland/scripting/pch.h"
#include "pland/scripting/system/ScriptTimerSystem.h"

#include "ll/api/mod/ModManagerRegistry.h"
#include <ll/api/mod/Mod.h>

namespace land::scripting {

constexpr std::string_view kPrivateScriptDir = "scripts";
constexpr std::string_view kEntryFileName    = "index.bin";

namespace {
bool gIsInitialized = false;
}

class ScriptingBootstrap::Impl {
public:
    ll::mod::Mod&                          mEntry;
    std::unique_ptr<ScriptTimerSystem>     mTimerSystem{nullptr};
    std::shared_ptr<CommunityPluginLoader> mCommunityPluginLoader{nullptr};
    std::unique_ptr<jspp::Engine>          mPrivateEngine{nullptr};

    ll::Expected<> initPrivateEngine() {
        auto entry     = mEntry.getModDir() / kPrivateScriptDir / kEntryFileName;
        mPrivateEngine = EngineHelper::newEngine(true);

        if (auto ok = EngineHelper::loadScript(entry, *mPrivateEngine, true); !ok) {
            mPrivateEngine.reset();
            mPrivateEngine = nullptr;
            return ok;
        }
        return {};
    }
};

ScriptingBootstrap::ScriptingBootstrap(ll::mod::Mod& mod) : impl(std::make_unique<Impl>(mod)) {}
ScriptingBootstrap::~ScriptingBootstrap() { (void)shutdown(); }

ScriptTimerSystem&                     ScriptingBootstrap::getScriptTimerSystem() const { return *impl->mTimerSystem; }
std::shared_ptr<CommunityPluginLoader> ScriptingBootstrap::getPluginLoader() const {
    return impl->mCommunityPluginLoader;
}

ll::Expected<> ScriptingBootstrap::initialize(bool initCommunityPluginLoader) {
    if (gIsInitialized) {
        return ll::makeStringError("scripting already initialized");
    }
    gIsInitialized = true;

    impl->mTimerSystem = std::make_unique<ScriptTimerSystem>();

    if (auto ok = impl->initPrivateEngine(); !ok) {
        return ok;
    }

    if (initCommunityPluginLoader) {
        // TODO: After the host (PLand) hot reloads, it can't re-enumerate plugins
        impl->mCommunityPluginLoader = std::make_shared<CommunityPluginLoader>();
        [[maybe_unused]] bool ok = ll::mod::ModManagerRegistry::getInstance().addManager(impl->mCommunityPluginLoader);
        assert(ok);
    }
    return {};
}
ll::Expected<> ScriptingBootstrap::shutdown() {
    if (!gIsInitialized) {
        return ll::makeStringError("scripting not initialized");
    }
    gIsInitialized = false;

    impl->mPrivateEngine.reset();

    if (impl->mCommunityPluginLoader) {
        if (!ll::mod::ModManagerRegistry::getInstance().eraseManager(impl->mCommunityPluginLoader->getType())) {
            [[unlikely]] return ll::makeStringError("failed to erase mod manager");
        }
        impl->mCommunityPluginLoader.reset();
    }

    impl->mTimerSystem.reset();
    return {};
}


ll::Expected<> ScriptingBootstrap::reloadPrivateEngine() {
    if (!gIsInitialized) [[unlikely]] {
        return ll::makeStringError("scripting not initialized");
    }
    // disable -> reload -> load -> enable
    return postPrivateEngineOnDisable()
        .and_then([&]() -> ll::Expected<> {
            impl->mPrivateEngine.reset();
            return {};
        })
        .and_then([&]() { return impl->initPrivateEngine(); })
        .and_then([&]() { return postPrivateEngineOnLoad(); })
        .and_then([&]() { return postPrivateEngineOnEnable(); });
}
ll::Expected<> ScriptingBootstrap::postPrivateEngineOnLoad() const {
    auto& engine = impl->mPrivateEngine;
    assert(engine);
    return EngineHelper::callScriptLifecycleHook(*engine, EngineHelper::LifecycleHookKind::kOnLoad);
}
ll::Expected<> ScriptingBootstrap::postPrivateEngineOnEnable() const {
    auto& engine = impl->mPrivateEngine;
    assert(engine);
    return EngineHelper::callScriptLifecycleHook(*engine, EngineHelper::LifecycleHookKind::kOnEnable);
}
ll::Expected<> ScriptingBootstrap::postPrivateEngineOnDisable() const {
    auto& engine = impl->mPrivateEngine;
    assert(engine);
    return EngineHelper::callScriptLifecycleHook(*engine, EngineHelper::LifecycleHookKind::kOnDisable);
}

} // namespace land::scripting