#include "pland/scripting/binding/Helper.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "ll/api/service/PlayerInfo.h"

namespace jspp::binding {

template <>
struct TypeConverter<ll::service::PlayerInfo::PlayerInfoEntry> {
    using type = ll::service::PlayerInfo::PlayerInfoEntry;
    static Local<Value> toJs(type const& value, ReturnValuePolicy /* policy */, Local<Value> const& /* parent */) {
        auto object = Object::newObject();
        object.set(String::newString("uuid"), binding::toJs(value.uuid));
        object.set(String::newString("xuid"), binding::toJs(value.xuid));
        object.set(String::newString("name"), binding::toJs(value.name));
        return object;
    }

    static type toCpp(Local<Value> const&) {
        [[unlikely]] throw Exception{
            "Cannot convert value to PlayerInfoEntry, that is not available convert",
            ExceptionType::Error
        };
    }
};
} // namespace jspp::binding

namespace land::scripting {

using ll::service::PlayerInfo;
decltype(Modules::kScriptPlayerInfo) Modules::kScriptPlayerInfo = //
    jspp::binding::defClass<void>("PlayerInfo")
        .func(
            "fromUuid",
            [](std::string const& rawUuid) -> jspp::Local<jspp::Value> {
                if (mce::UUID::canParse(rawUuid)) {
                    auto uuid = mce::UUID::fromString(rawUuid);
                    if (auto info = PlayerInfo::getInstance().fromUuid(uuid)) {
                        return jspp::binding::toJs(*info);
                    }
                }
                return jspp::Null::newNull();
            }
        )
        .func(
            "fromXuid",
            [](std::string const& xuid) -> jspp::Local<jspp::Value> {
                if (auto info = PlayerInfo::getInstance().fromXuid(xuid)) {
                    return jspp::binding::toJs(*info);
                }
                return jspp::Null::newNull();
            }
        )
        .func(
            "fromName",
            [](std::string const& name) -> jspp::Local<jspp::Value> {
                if (auto info = PlayerInfo::getInstance().fromName(name)) {
                    return jspp::binding::toJs(*info);
                }
                return jspp::Null::newNull();
            }
        )
        .build();

} // namespace land::scripting