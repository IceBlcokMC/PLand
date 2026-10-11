#pragma once
#include <memory>

#include "ll/api/Expected.h"

namespace ll::mod {
class Mod;
}

namespace land::scripting {

class EngineManager;
class CommunityPluginLoader;
class ScriptTimerSystem;

class ScriptingBootstrap {
    class Impl;
    std::unique_ptr<Impl> impl{nullptr};

public:
    explicit ScriptingBootstrap(ll::mod::Mod& mod);
    ~ScriptingBootstrap();

    [[nodiscard]] ScriptTimerSystem&                     getScriptTimerSystem() const;
    [[nodiscard]] std::shared_ptr<CommunityPluginLoader> getPluginLoader() const;

    // initialize / shutdown scripting sub system
    [[nodiscard]] ll::Expected<> initialize(bool initCommunityPluginLoader = true);
    [[nodiscard]] ll::Expected<> shutdown();

    // private engine
    [[nodiscard]] ll::Expected<> reloadPrivateEngine();
    [[nodiscard]] ll::Expected<> postPrivateEngineOnLoad() const;
    [[nodiscard]] ll::Expected<> postPrivateEngineOnEnable() const;
    [[nodiscard]] ll::Expected<> postPrivateEngineOnDisable() const;
};

} // namespace land::scripting