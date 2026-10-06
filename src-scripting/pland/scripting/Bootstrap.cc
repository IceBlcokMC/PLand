#include "Bootstrap.h"

#include "EngineOwnData.h"
#include "binding/Modules.h"
#include "pch.h"
#include "system/ScriptTimerSystem.h"

#include <ll/api/mod/Mod.h>

namespace land::scripting {

constexpr std::string_view kScriptDir     = "scripts";
constexpr std::string_view kEntryFileName = "index.bin";

constexpr std::string_view kOnLoadMethod    = "onLoad";
constexpr std::string_view kOnEnableMethod  = "onEnable";
constexpr std::string_view kOnDisableMethod = "onDisable";

namespace fs = std::filesystem;

class Bootstrap::Impl {
    ll::mod::Mod&                      mod;
    bool                               isInitialized{false};
    std::unique_ptr<jspp::Engine>      engine{nullptr};
    std::unique_ptr<ScriptTimerSystem> timerSystem{nullptr};

    /// 实例化模块命名空间的 default 导出 (入口类), 结果存入 scriptMod
    ll::Expected<> resolveDefaultExportObjectAndInit(jspp::Local<jspp::Object> const& ns) {
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

        this->engine->getData<EngineOwnData>()->scriptMod.reset(inst.asObject());
        return {};
    }

public:
    explicit Impl(ll::mod::Mod& mod_) : mod(mod_) {}

    ScriptTimerSystem& getScriptTimerSystem() { return *timerSystem; }

    ll::Expected<> performInitialize() {
        if (isInitialized) {
            return ll::makeStringError("Engine already initialized");
        }

        auto file = mod.getModDir() / kScriptDir / kEntryFileName;
        if (!fs::exists(file)) {
            return ll::makeStringError("script not found: " + file.string());
        }

        engine = std::make_unique<jspp::Engine>();
        engine->setData(std::make_shared<EngineOwnData>()); // managed by jspp

        timerSystem = std::make_unique<ScriptTimerSystem>();

        auto _ = jspp::EngineScope{*engine};
        try {
            Modules::bind(*engine);

            // loadModule 返回入口模块的命名空间对象, default 导出即脚本入口类
            auto ns = engine->loadByteCodeNamespace(file, true);
            return resolveDefaultExportObjectAndInit(ns);
        } catch (jspp::Exception const& e) {
            return ll::makeStringError(fmt::format("Failed to load script: {}\n{}", e.what(), e.stacktrace()));
        }
    }
    ll::Expected<> performShutdown() {
        if (!isInitialized) {
            return {};
        }
        this->timerSystem.reset();
        this->engine.reset();
        this->timerSystem   = nullptr;
        this->engine        = nullptr;
        this->isInitialized = false;
        return {};
    }

    [[nodiscard]] ll::Expected<> callScriptModMethod(std::string_view method) const {
        auto _ = jspp::EngineScope{*engine};
        try {
            auto v = this->engine->getData<EngineOwnData>()->scriptMod.get();

            auto maybe_fn = v.get(jspp::String::newString(method));
            if (!maybe_fn.isFunction()) {
                return ll::makeStringError(fmt::format("'{}' is not a function", method));
            }
            (void)maybe_fn.asFunction().call(v);
            return {};
        } catch (jspp::Exception const& e) {
            return ll::makeStringError(fmt::format("failed to call 'onLoad' method: {}\n{}", e.what(), e.stacktrace()));
        }
    }
};

Bootstrap::Bootstrap(ll::mod::Mod& mod) : impl(std::make_unique<Impl>(mod)) {}
Bootstrap::~Bootstrap() { (void)shutdown(); }

ScriptTimerSystem& Bootstrap::getScriptTimerSystem() { return impl->getScriptTimerSystem(); }

ll::Expected<> Bootstrap::initialize() { return impl->performInitialize(); }
ll::Expected<> Bootstrap::shutdown() { return impl->performShutdown(); }

ll::Expected<> Bootstrap::reload() {
    (void)shutdown();
    return initialize();
}

ll::Expected<> Bootstrap::postOnLoad() const { return impl->callScriptModMethod(kOnLoadMethod); }
ll::Expected<> Bootstrap::postOnEnable() const { return impl->callScriptModMethod(kOnEnableMethod); }
ll::Expected<> Bootstrap::postOnDisable() const { return impl->callScriptModMethod(kOnDisableMethod); }

} // namespace land::scripting