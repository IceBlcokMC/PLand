#pragma once
#include "pch.h"

#include "ll/api/coro/ConditionVariable.h"

namespace land::scripting {

class CommunityScriptPlugin;
struct EventLoopControl; // forward declaration

struct EngineOwnData final {
    jspp::Engine*              self{nullptr};
    bool const                 isPrivateEngine{false}; // only PLand bult-in logic engine trusted
    jspp::Global<jspp::Object> scriptEntry{};          // script default export's class instance

    std::string                            communityPluginName{};          // only community plugin
    std::shared_ptr<CommunityScriptPlugin> communityScriptPlugin{nullptr}; // only community plugin

    std::shared_ptr<EventLoopControl> eventLoopControl{nullptr};

    std::vector<uint32_t> runningTimers{};

    EngineOwnData() = default;
    EngineOwnData(jspp::Engine* self_, bool ispriv) : self(self_), isPrivateEngine(ispriv) {}
    ~EngineOwnData(); // call on engine destruction
};

struct EventLoopControl final {
    ll::coro::ConditionVariable cv;
    std::atomic_bool            abort{false};
};

struct EngineHelper {
    EngineHelper() = delete;

    [[nodiscard]] static std::unique_ptr<jspp::Engine> newEngine(bool trust = false);

    [[nodiscard]] static ll::Expected<>
    loadScript(std::filesystem::path const& entry, jspp::Engine& engine, bool bytecode = false);

    enum class LifecycleHookKind { kOnLoad, kOnEnable, kOnDisable };
    [[nodiscard]] static ll::Expected<> callScriptLifecycleHook(jspp::Engine& engine, LifecycleHookKind kind);

    static ll::Expected<> launchEventLoopFor(jspp::Engine& engine);
};

} // namespace land::scripting
