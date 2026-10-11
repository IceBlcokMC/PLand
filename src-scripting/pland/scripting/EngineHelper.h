#pragma once
#include "pch.h"

namespace land::scripting {

class CommunityScriptPlugin;

struct EngineOwnData {
    bool const                 isPrivateEngine{false}; // only PLand bult-in logic engine trusted
    jspp::Global<jspp::Object> scriptEntry{};          // script default export's class instance

    std::string                            communityPluginName{};          // only community plugin
    std::shared_ptr<CommunityScriptPlugin> communityScriptPlugin{nullptr}; // only community plugin

    EngineOwnData() = default;
    EngineOwnData(bool ispriv) : isPrivateEngine(ispriv) {}
};

struct EngineHelper {
    EngineHelper() = delete;

    [[nodiscard]] static std::unique_ptr<jspp::Engine> newEngine(bool trust = false);

    [[nodiscard]] static ll::Expected<>
    loadScript(std::filesystem::path const& entry, jspp::Engine& engine, bool bytecode = false);

    enum class LifecycleHookKind { kOnLoad, kOnEnable, kOnDisable };
    [[nodiscard]] static ll::Expected<> callScriptLifecycleHook(jspp::Engine& engine, LifecycleHookKind kind);
};

} // namespace land::scripting
