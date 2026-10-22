#include "pland/scripting/binding/Helper.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "pland/PLand.h"
#include "pland/scripting/EngineHelper.h"
#include "pland/scripting/community/CommunityScriptPlugin.h"

#include <ll/api/io/LogLevel.h>
#include <ll/api/io/Logger.h>

namespace land::scripting {

namespace {

auto& getLoggerInstance() {
    auto data = jspp::EngineScope::currentEngineChecked().getData<EngineOwnData>();
    if (data->isPrivateEngine) {
        return PLand::getInstance().getSelf().getLogger();
    } else {
        assert(data->communityScriptPlugin != nullptr);
        return data->communityScriptPlugin->getLogger();
    }
}

template <size_t fmt_start_offset = 0>
void multiArgsLogImpl(jspp::Arguments const& args, ll::io::LogLevel level) {
    auto argc = args.length();
    if (argc == 0) [[unlikely]] {
        return; // log empty ???
    }

    auto& logger = getLoggerInstance();
    if (argc == 1) {
        logger.log(level, args[fmt_start_offset].toString().getValue());
    } else if (argc >= 2) {
        auto msg = helper::format<fmt_start_offset>(args);
        logger.log(level, std::move(msg));
    }
}

template <ll::io::LogLevel L>
jspp::Local<jspp::Value> multiArgsLogHelper(jspp::Arguments const& args) {
    static_assert(L != ll::io::LogLevel::Off, "Cannot log with level Off");
    multiArgsLogImpl(args, L);
    return {};
}

jspp::Local<jspp::Value> runtimeMultiArgsLogHelper(jspp::Arguments const& args) {
    if (args.length() < 2) [[unlikely]] {
        throw jspp::Exception{"Too few arguments for log function", jspp::ExceptionType::TypeError};
    }
    auto level = jspp::binding::toCpp<ll::io::LogLevel>(args[0]);
    multiArgsLogImpl<1>(args, level);
    return {};
}

static_assert(jspp::binding::traits::isFunctionCallback_v<decltype(&multiArgsLogHelper<ll::io::LogLevel::Debug>)>);
static_assert(jspp::binding::traits::isFunctionCallback_v<decltype(&runtimeMultiArgsLogHelper)>);

} // namespace

using ll::io::Logger;
decltype(Modules::kScriptLogger) Modules::kScriptLogger = //
    jspp::binding::defClass<void>("logger")
        .instanceProvider(getLoggerInstance)
        .func("log", &runtimeMultiArgsLogHelper)
        .func("info", &multiArgsLogHelper<ll::io::LogLevel::Info>)
        .func("warn", &multiArgsLogHelper<ll::io::LogLevel::Warn>)
        .func("error", &multiArgsLogHelper<ll::io::LogLevel::Error>)
        .func("fatal", &multiArgsLogHelper<ll::io::LogLevel::Fatal>)
        .func("debug", &multiArgsLogHelper<ll::io::LogLevel::Debug>)
        .func("trace", &multiArgsLogHelper<ll::io::LogLevel::Trace>)
        .func("getTitle", &Logger::getTitle)
        .func("getLevel", &Logger::getLevel)
        .func("shouldLog", &Logger::shouldLog)
        .func("setLevel", &Logger::setLevel)
        .build();

using ll::io::LogLevel;
decltype(Modules::kScriptLogLevel) Modules::kScriptLogLevel = //
    helper::auto_gen_enum_def<LogLevel>("LogLevel");

} // namespace land::scripting