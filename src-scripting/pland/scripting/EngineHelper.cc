#include "EngineHelper.h"
#include "ScriptingBootstrap.h"
#include "binding/Modules.h"

#include "pland/PLand.h"
#include "pland/scripting/system/InteropHub.h"
#include "pland/scripting/system/ScriptTimerSystem.h"

#include "ll/api/event/EventBus.h"
#include <ll/api/Expected.h>
#include <ll/api/chrono/GameChrono.h>
#include <ll/api/coro/ConditionVariable.h>
#include <ll/api/coro/CoroTask.h>
#include <ll/api/thread/ServerThreadExecutor.h>

#include "jspp-backend/queue/JobQueue.h" // for event loop (JobQueue)

namespace land::scripting {

EngineOwnData::~EngineOwnData() {
    auto& bootstrap = PLand::getInstance().getScriptingBootstrap();
    {
        if (auto control = this->eventLoopControl) {
            control->abort.store(true);
            control->cv.notifyAll(true); // wake up the event loop
        }
    }
    {
        // clang-format off
        //
        // script: eventBus.unsubscribe() -> native: ScriptEventBus::unsubscribe -> erase eventListeners by id
        //                                                                     |
        //                                                                     |----------------------|--> EventBus::removeListener(id)
        //                                                                                            /
        // native: ~Engine -> shared_ptr<>::reset() -> ~EngineOwnData -> clear all event listeners --/
        //
        // clang-format on
        auto& bus = ll::event::EventBus::getInstance();
        for (auto id : this->eventListeners) {
            bool ok = bus.removeListener(id);
            assert(ok);
        }
    }
    {
        auto& timer = bootstrap.getScriptTimerSystem();

        auto copy = this->runningTimers; // copy to avoid iterator invalidation
        for (auto id : copy) {
            timer.cancelTask(id); // stop all running timers
        }

        // clang-format off
        //                                                /-> native layer:cancelTask -> Coro Exit -> Finalizer -> isDestroying ? skip : erase runningTimers by id
        // script layer: clearTimeout/clearInterval(id) -/         ↑                                                                     |
        //                                                         \------------------------|                                           /
        // ~Engine -> shared_ptr<>::reset() -> ~EngineOwnData -> cancel all running timers -/                                          /
        //    ↑                                                                                                                       /
        //    \------------ std::erase(getData<EngineOwnData>->runningTimers, id) ---------------------------------------------------/
        // clang-format on
    }
    {
        auto& hub = bootstrap.getInteropHub();
        hub.clearExports(this->self); // clear all interop exports
    }
}

std::unique_ptr<jspp::Engine> EngineHelper::newEngine(bool trust) {
    auto engine = std::make_unique<jspp::Engine>();
    engine->setData(std::make_shared<EngineOwnData>(engine.get(), trust));
    Modules::bind(*engine);
    return engine;
}

ll::Expected<> EngineHelper::loadScript(std::filesystem::path const& entry, jspp::Engine& engine, bool bytecode) {
    namespace fs = std::filesystem;
    if (!fs::exists(entry)) {
        return ll::makeStringError("script not found: " + entry.string());
    }
    auto _ = jspp::EngineScope{engine};
    try {
        auto ns = [&] -> jspp::Local<jspp::Object> {
            if (bytecode) {
                return engine.loadByteCodeContainerNamespace(entry, true);
            } else {
                return engine.loadModule(entry);
            }
        }();

        auto exports = ns.get(jspp::String::newString("default"));
        if (!exports.isFunction()) {
            return ll::makeStringError(
                fmt::format(
                    "entry file must default export a class, got {}",
                    exports.isUndefined() ? "undefined (no default export)" : exports.toString().getValue()
                )
            );
        }

        auto ctor = exports.asFunction();
        if (!ctor.isConstructor()) {
            return ll::makeStringError("entry file's default export must be a class");
        }

        auto inst = ctor.callAsConstructor();
        if (!inst.isObject()) {
            return ll::makeStringError(fmt::format("'{}' is not a stable class", ctor.toString().getValue()));
        }

        engine.getData<EngineOwnData>()->scriptEntry.reset(inst.asObject());
        return {};
    } catch (jspp::Exception const& e) {
        return ll::makeStringError(fmt::format("Failed to load script: {}\n{}", e.what(), e.stacktrace()));
    }
}

ll::Expected<> EngineHelper::callScriptLifecycleHook(jspp::Engine& engine, LifecycleHookKind kind) {
    static constexpr std::string_view kOnLoadMethod    = "onLoad";
    static constexpr std::string_view kOnEnableMethod  = "onEnable";
    static constexpr std::string_view kOnDisableMethod = "onDisable";

    std::string_view method{};
    switch (kind) {
    case LifecycleHookKind::kOnLoad:
        method = kOnLoadMethod;
        break;
    case LifecycleHookKind::kOnEnable:
        method = kOnEnableMethod;
        break;
    case LifecycleHookKind::kOnDisable:
        method = kOnDisableMethod;
        break;
    }

    auto _ = jspp::EngineScope{engine};
    try {
        auto v = engine.getData<EngineOwnData>()->scriptEntry.get();

        auto maybe_fn = v.get(jspp::String::newString(method));
        if (!maybe_fn.isFunction()) {
            return ll::makeStringError(fmt::format("'{}' is not a function", method));
        }
        (void)maybe_fn.asFunction().call(v);
        return {};
    } catch (jspp::Exception const& e) {
        return ll::makeStringError(
            fmt::format("failed to call '{}' method:\n{}\n{}", method, e.what(), e.stacktrace())
        );
    }
}

ll::Expected<> EngineHelper::launchEventLoopFor(jspp::Engine& engine) {
    auto data = engine.getData<EngineOwnData>();
    assert(data != nullptr); // not null

    if (data->eventLoopControl != nullptr) {
        return ll::makeStringError("event loop already running");
    }

    data->eventLoopControl = std::make_shared<EventLoopControl>();
    ll::coro::keepThis([engine = &engine, control = data->eventLoopControl]() -> ll::coro::CoroTask<> {
        while (!control->abort.load()) {
            co_await control->cv.waitFor(ll::chrono::ticks{1});
            if (control->abort.load()) {
                break;
            }
            try {
                engine->getJobQueue()->loopOnce(); // pumpPendingJobs internally entered EngineScope
            } catch (jspp::Exception const& e) {
                PLand::getInstance().getSelf().getLogger().error(
                    "An uncaught jspp::Exception occurred in the event loop: {}\n{}",
                    e.what(),
                    e.stacktrace()
                );
            } catch (std::exception const& e) {
                PLand::getInstance().getSelf().getLogger().error(
                    "An uncaught std::exception occurred in the event loop: {}",
                    e.what()
                );
            } catch (...) {
                PLand::getInstance().getSelf().getLogger().error("An uncaught exception occurred in the event loop");
            }
        }
        co_return;
    }).launch(ll::thread::ServerThreadExecutor::getDefault());
    return {};
}


} // namespace land::scripting