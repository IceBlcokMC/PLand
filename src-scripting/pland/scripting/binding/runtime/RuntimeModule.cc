#include "pland/PLand.h"
#include "pland/scripting/Bootstrap.h"
#include "pland/scripting/EngineOwnData.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"
#include "pland/scripting/system/ScriptTimerSystem.h"

#include "fmt/format.h"

namespace land::scripting {

namespace {

std::string formatImpl(jspp::Arguments const& args) {
    auto len = args.length();
    if (len == 0) [[unlikely]] {
        throw jspp::Exception{"log/format requires at least one argument", jspp::ExceptionType::TypeError};
    }

    auto f       = args[0];
    auto fmt_str = f.asString().getValue();

    fmt::dynamic_format_arg_store<fmt::format_context> store;
    for (int i = 1; i < len; ++i) {
        auto v = args[i];
        store.push_back(v.toString().getValue());
    }
    return fmt::vformat(fmt_str, store);
}

jspp::Local<jspp::Value> format(jspp::Arguments const& args) { return jspp::binding::toJs(formatImpl(args)); }

jspp::Local<jspp::Value> log(jspp::Arguments const& args) {
    auto result = formatImpl(args);
    fmt::print("{}\n", result);
    return {};
}

jspp::Local<jspp::Value> self(jspp::Arguments const& arguments) {
    if (auto engine = arguments.runtime()) {
        auto data = engine->getData<EngineOwnData>();
        return data->scriptMod.get();
    }
    return jspp::Null::newNull();
}


jspp::Local<jspp::Value> setTimerImpl(jspp::Arguments const& arguments, bool interval) {
    if (arguments.length() != 2) [[unlikely]] {
        throw jspp::Exception{"setTimeout/setInterval requires two arguments", jspp::ExceptionType::TypeError};
    }

    auto maybe_fn = arguments[0];
    if (!maybe_fn.isFunction()) [[unlikely]] {
        throw jspp::Exception{
            "setTimeout/setInterval first argument must be a function",
            jspp::ExceptionType::TypeError
        };
    }

    auto maybe_timeout = arguments[1];
    if (!maybe_timeout.isNumber()) [[unlikely]] {
        throw jspp::Exception{
            "setTimeout/setInterval second argument must be a number",
            jspp::ExceptionType::TypeError
        };
    }

    auto fn      = maybe_fn.asFunction();
    auto timeout = maybe_timeout.asNumber().getInt32();

    auto& timer = PLand::getInstance().getScriptingBootstrap().getScriptTimerSystem();

    auto id = interval ? timer.newInterval(fn, timeout) : timer.newTimeout(fn, timeout);
    return jspp::binding::toJs(id);
}

jspp::Local<jspp::Value> setTimeout(jspp::Arguments const& arguments) { return setTimerImpl(arguments, false); }
jspp::Local<jspp::Value> setInterval(jspp::Arguments const& arguments) { return setTimerImpl(arguments, true); }
bool                     clearTimeout(uint32_t id) {
    auto& timer = PLand::getInstance().getScriptingBootstrap().getScriptTimerSystem();
    return timer.cancelTask(id);
}

} // namespace

jspp::ModuleMeta const& Modules::getUtilsModule() {
    static jspp::ModuleMeta m = jspp::binding::defModule("@runtime") //
                                    .exportFunction("log", &log)
                                    .exportFunction("format", &format)
                                    .exportFunction("self", &self)
                                    .exportFunction("setTimeout", &setTimeout)
                                    .exportFunction("setInterval", &setInterval)
                                    .exportFunction("clearTimeout", &clearTimeout)
                                    .exportFunction("clearInterval", &clearTimeout)
                                    .build();
    return m;
}

} // namespace land::scripting