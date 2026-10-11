#include "EngineHelper.h"

#include "EngineHelper.h"
#include "binding/Modules.h"

namespace land::scripting {

std::unique_ptr<jspp::Engine> EngineHelper::newEngine(bool trust) {
    auto engine = std::make_unique<jspp::Engine>();
    engine->setData(std::make_shared<EngineOwnData>(trust));
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
                return engine.loadByteCodeNamespace(entry, true);
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

} // namespace land::scripting