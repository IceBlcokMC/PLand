#pragma once
#include "pch.h"

#include "ll/api/coro/ConditionVariable.h"

namespace ll::mod {
class Mod;
class NativeMod;
} // namespace ll::mod

namespace land::scripting {

struct EventLoopControl; // forward declaration

struct EngineOwnData final {
    jspp::Engine*              self{nullptr};          // engine instance(for cleanup)
    bool const                 isPrivateEngine{false}; // only PLand bult-in logic engine trusted
    jspp::Global<jspp::Object> scriptEntry{};          // script default export's class instance

    // if isPrivateEngine is true, then pluginInst is same as self(aka PLand's mod instance)
    // else pluginInst is a CommunityScriptPlugin instance
    std::shared_ptr<ll::mod::Mod> pluginInst{nullptr}; // plugin/mod instance

    std::shared_ptr<EventLoopControl> eventLoopControl{nullptr};

    std::vector<uint32_t> runningTimers{};

    std::vector<uint64_t> eventListeners{};

    explicit EngineOwnData() = delete;
    explicit EngineOwnData(jspp::Engine* self_, bool ispriv, std::shared_ptr<ll::mod::Mod> modInst)
    : self(self_),
      isPrivateEngine(ispriv),
      pluginInst(std::move(modInst)) {}

    ~EngineOwnData(); // call on engine destruction
};

struct EventLoopControl final {
    ll::coro::ConditionVariable cv;
    std::atomic_bool            abort{false};
};

struct EngineHelper final {
    EngineHelper() = delete;

    [[nodiscard]] static std::unique_ptr<jspp::Engine>
    newEngine(bool ispriv = false, std::shared_ptr<ll::mod::NativeMod> const& modInst = nullptr);

    [[nodiscard]] static ll::Expected<>
    loadScript(std::filesystem::path const& entry, jspp::Engine& engine, bool bytecode = false);

    enum class LifecycleHookKind { kOnLoad, kOnEnable, kOnDisable };
    [[nodiscard]] static ll::Expected<> callScriptLifecycleHook(jspp::Engine& engine, LifecycleHookKind kind);

    static ll::Expected<> launchEventLoopFor(jspp::Engine& engine);
};

} // namespace land::scripting
