#include "pland/scripting/binding/Helper.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "ll/api/i18n/I18n.h"

namespace land::scripting {

namespace {

using ll::i18n::I18n;

// tr(key, ...params?)
// trl(localeCode, key, ...params?)

template <bool requriedLocaleCode = false>
std::string resolveLocaleCode(jspp::Arguments const& args) {
    if constexpr (requriedLocaleCode) {
        return jspp::binding::toCpp<std::string>(args[0]); // first argument is localeCode
    } else {
        return std::string{ll::i18n::getDefaultLocaleCode()};
    }
}

template <bool requriedLocaleCode = false>
jspp::Local<jspp::Value> translate(jspp::InstancePayload& payload, jspp::Arguments const& args) {
    constexpr auto offset = requriedLocaleCode ? 1 : 0; // skip localeCode argument

    if (args.length() <= offset) [[unlikely]] {
        throw jspp::Exception{
            requriedLocaleCode ? "trl requires at least 2 arguments" : "tr requires at least 1 argument",
            jspp::ExceptionType::TypeError
        };
    }

    auto localeCode = resolveLocaleCode<requriedLocaleCode>(args);
    auto key        = jspp::binding::toCpp<std::string>(args[offset]);

    auto i18n = payload.unwrap<I18n>();
    assert(i18n != nullptr); // not null

    constexpr auto params_begin = offset + 1; // skip key argument

    auto text = i18n->get(key, localeCode);
    auto res  = helper::formatImpl<params_begin>(text, args);
    return jspp::binding::toJs(res);
}

static_assert(jspp::binding::traits::isInstanceMethodCallback_v<decltype(translate<false>)>);

} // namespace


decltype(Modules::kScriptI18n) Modules::kScriptI18n = //
    jspp::binding::defClass<I18n>("I18n")
        .ctor()
        .method(
            "load",
            [](I18n& self, std::filesystem::path const& path) {
                if (auto expected = self.load(path); !expected) {
                    throw jspp::Exception{expected.error().message()};
                }
            }
        )
        .method("clear", &I18n::clear)
        .method("get", &I18n::get)
        .method("set", &I18n::set)
        .method("tr", translate<false>)
        .method("trl", translate<true>)
        .var_readonly("defaultLocaleCode", ll::i18n::getDefaultLocaleCode)
        .build();

} // namespace land::scripting