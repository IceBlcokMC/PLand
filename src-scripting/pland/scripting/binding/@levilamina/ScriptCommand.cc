#include "pland/scripting/binding/Helper.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "mc/world/level/Level.h"
#include <mc/world/actor/player/Player.h>

#include "ll/api/service/Bedrock.h"

#include "ll/api/command/Command.h"
#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/Overload.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "pland/PLand.h"

namespace jspp::binding::traits {

template <>
constexpr bool isReferenceOnlyType<ll::command::CommandRegistrar> = true;

template <>
constexpr bool isReferenceOnlyType<ll::command::CommandHandle> = true;

} // namespace jspp::binding::traits

namespace land ::scripting {
using ll::command::CommandRegistrar;
decltype(Modules::kScriptCommandRegistrar) Modules::kScriptCommandRegistrar = //
    jspp::binding::defClass<CommandRegistrar>("CommandRegistrar")
        .func(
            "getInstance",
            []() -> auto& { return CommandRegistrar::getInstance(false); },
            jspp::binding::ReturnValuePolicy::kReferencePersistent
        )
        .ctor(nullptr)
        .method(
            "getOrCreateCommand",
            [](CommandRegistrar& self, std::string const& name) -> auto& { return self.getOrCreateCommand(name); },
            [](CommandRegistrar& self, std::string const& name, std::string const& desc) -> auto& {
                return self.getOrCreateCommand(name, desc);
            },
            [](CommandRegistrar& self, std::string const& name, std::string const& desc, CommandPermissionLevel level)
                -> auto& { return self.getOrCreateCommand(name, desc, level); },
            jspp::binding::ReturnValuePolicy::kReferencePersistent // CommandHandle&
        )
        .method("hasEnum", &CommandRegistrar::hasEnum)
        .method(
            "tryRegisterRuntimeEnum",
            static_cast<bool (CommandRegistrar::*)(std::string const&, std::vector<std::pair<std::string, uint64>>)>(
                &CommandRegistrar::tryRegisterRuntimeEnum
            )
        )
        .method("addRuntimeEnumValues", &CommandRegistrar::addRuntimeEnumValues)
        .method("hasSoftEnum", &CommandRegistrar::hasSoftEnum)
        .method(
            "tryRegisterSoftEnum",
            static_cast<bool (CommandRegistrar::*)(std::string const&, std::vector<std::string>)>(
                &CommandRegistrar::tryRegisterSoftEnum
            )
        )
        .method("addSoftEnumValues", &CommandRegistrar::addSoftEnumValues)
        .method("removeSoftEnumValues", &CommandRegistrar::removeSoftEnumValues)
        .method("setSoftEnumValues", &CommandRegistrar::setSoftEnumValues)
        .build();


using ll::command::CommandHandle;
using ll::command::ParamKindType;
using ll::command::RuntimeOverload;
using CommandParamKind = ll::command::ParamKind::Kind;
class ScriptRuntimeOverloadProxy {
    struct ParamData {
        std::string      name;
        CommandParamKind kind;
        std::string      enumName;
    };
    RuntimeOverload        impl;
    std::vector<ParamData> params;

public:
    explicit ScriptRuntimeOverloadProxy(RuntimeOverload&& overload) : impl(std::move(overload)) {}

    ScriptRuntimeOverloadProxy(ScriptRuntimeOverloadProxy const&) = delete;
    ScriptRuntimeOverloadProxy(ScriptRuntimeOverloadProxy&& other) noexcept
    : impl(std::move(other.impl)),
      params(std::move(other.params)) {}

    ScriptRuntimeOverloadProxy& optional(std::string_view name, ParamKindType kind) {
        (void)impl.optional(name, kind);
        params.emplace_back(std::string{name}, static_cast<CommandParamKind>(kind));
        return *this;
    }
    ScriptRuntimeOverloadProxy& required(std::string_view name, ParamKindType kind) {
        (void)impl.required(name, kind);
        params.emplace_back(std::string{name}, static_cast<CommandParamKind>(kind));
        return *this;
    }
    ScriptRuntimeOverloadProxy& optional(std::string_view name, ParamKindType enumKind, std::string_view enumName) {
        (void)impl.optional(name, enumKind, enumName);
        params.emplace_back(std::string{name}, static_cast<CommandParamKind>(enumKind), std::string{enumName});
        return *this;
    }
    ScriptRuntimeOverloadProxy& required(std::string_view name, ParamKindType enumKind, std::string_view enumName) {
        (void)impl.required(name, enumKind, enumName);
        params.emplace_back(std::string{name}, static_cast<CommandParamKind>(enumKind), std::string{enumName});
        return *this;
    }
    ScriptRuntimeOverloadProxy& text(std::string_view text) {
        (void)impl.text(text);
        return *this;
    }
    ScriptRuntimeOverloadProxy& postfix(std::string_view postfix) {
        (void)impl.postfix(postfix);
        return *this;
    }
    ScriptRuntimeOverloadProxy& option(CommandParameterOption option) {
        (void)impl.option(option);
        return *this;
    }
    ScriptRuntimeOverloadProxy& deoption(CommandParameterOption option) {
        (void)impl.deoption(option);
        return *this;
    }
    void execute(jspp::Arguments const& arguments) {
        auto len = arguments.length();
        if (len != 1 || !arguments[0].isFunction()) [[unlikely]] {
            throw jspp::Exception{"RuntimeOverload.execute required a callback", jspp::ExceptionType::TypeError};
        }
        auto fn      = arguments[0].asFunction();
        auto tracked = jspp::TrackedGlobal<jspp::Function>::create(std::move(fn));

        impl.execute([tracked, engine = arguments.runtime(), params = std::move(params)](
                         CommandOrigin const&               origin,
                         CommandOutput&                     output,
                         ll::command::RuntimeCommand const& cmd
                     ) {
            jspp::EngineScope          lock{engine};
            jspp::TransientObjectScope _{};

            auto& persent = tracked->global();
            auto  fn      = persent.get();

            auto sori = jspp::binding::toJs(origin, jspp::binding::ReturnValuePolicy::kReference, {});
            auto sout = jspp::binding::toJs(output, jspp::binding::ReturnValuePolicy::kReference, {});
            auto args = convertArgs(cmd, params, origin);
            try {
                jspp::binding::call(fn, {}, sori, sout, args);
            } catch (jspp::Exception const& e) {
                PLand::getInstance().getSelf().getLogger().error(
                    "Failed to execute command callback: {}\n{}",
                    e.what(),
                    e.stacktrace()
                );
            }
        });
    }

private:
    static jspp::Local<jspp::Object> convertArgs(
        ll::command::RuntimeCommand const& cmd,
        std::vector<ParamData> const&      params,
        CommandOrigin const&               origin
    ) {
        auto res = jspp::Object::newObject();
        for (auto const& [name, kind, enumName] : params) {
            try {
                auto& arg = cmd[name];
                res.set(jspp::String::newString(name), convertImpl(arg, origin));
            } catch (std::out_of_range&) {}
        }
        return res;
    }
    static jspp::Local<jspp::Value>
    convertImpl(ll::command::ParamStorageType const& storage, CommandOrigin const& origin) {
        if (!storage.has_value()) {
            return jspp::Null::newNull();
        }
        if (storage.hold(CommandParamKind::Enum)) {
            return jspp::binding::toJs(std::get<ll::command::RuntimeEnum>(storage.value()).index);
        }
        if (storage.hold(CommandParamKind::SoftEnum)) {
            return jspp::binding::toJs(std::get<ll::command::RuntimeSoftEnum>(storage.value()));
        }
        if (storage.hold(CommandParamKind::Int)) {
            return jspp::binding::toJs(std::get<int>(storage.value()));
        }
        if (storage.hold(CommandParamKind::Bool)) {
            return jspp::binding::toJs(std::get<bool>(storage.value()));
        }
        if (storage.hold(CommandParamKind::Float)) {
            return jspp::binding::toJs(std::get<float>(storage.value()));
        }
        if (storage.hold(CommandParamKind::String)) {
            return jspp::binding::toJs(std::get<std::string>(storage.value()));
        }
        if (storage.hold(CommandParamKind::Player)) {
            auto players = std::get<CommandSelector<Player>>(storage.value()).results(origin);
            auto array   = jspp::Array::newArray(players.size());
            for (auto const& player : players) {
                array.push(jspp::binding::toJs(player, jspp::binding::ReturnValuePolicy::kReferencePersistent, {}));
            }
            return array;
        }
        if (storage.hold(CommandParamKind::BlockPos)) {
            return jspp::binding::toJs(
                std::get<CommandPosition>(storage.value())
                    .getBlockPos(static_cast<int>(CurrentCmdVersion::Latest), origin, Vec3::ZERO())
            );
        }
        if (storage.hold(CommandParamKind::Vec3)) {
            return jspp::binding::toJs(
                std::get<CommandPosition>(storage.value())
                    .getPosition(static_cast<int>(CurrentCmdVersion::Latest), origin, Vec3::ZERO())
            );
        }
        if (storage.hold(CommandParamKind::RawText)) {
            return jspp::binding::toJs(std::get<CommandRawText>(storage.value()).mText);
        }
        return {};
    }
};

decltype(Modules::kScriptCommandHandle) Modules::kScriptCommandHandle = //
    jspp::binding::defClass<CommandHandle>("CommandHandle")
        .ctor(nullptr)
        .method(
            "runtimeOverload",
            [](CommandHandle& h) { return ScriptRuntimeOverloadProxy{h.runtimeOverload()}; },
            jspp::binding::ReturnValuePolicy::kMove
        )
        .method(
            "addAlias",
            static_cast<CommandHandle& (CommandHandle::*)(std::string_view)>(&CommandHandle::alias),
            jspp::binding::ReturnValuePolicy::kReferencePersistent
        )
        .method("getAliases", static_cast<std::vector<std::string> (CommandHandle::*)() const>(&CommandHandle::alias))
        .build();

decltype(Modules::kScriptRuntimeOverload) Modules::kScriptRuntimeOverload = //
    jspp::binding::defClass<ScriptRuntimeOverloadProxy>("RuntimeOverload")
        .ctor(nullptr)
        .method(
            "optional",
            static_cast<ScriptRuntimeOverloadProxy& (ScriptRuntimeOverloadProxy::*)(std::string_view, ParamKindType)>(
                &ScriptRuntimeOverloadProxy::optional
            ),
            static_cast<ScriptRuntimeOverloadProxy& (ScriptRuntimeOverloadProxy::*)(std::string_view,
                                                                                    ParamKindType,
                                                                                    std::string_view)>(
                &ScriptRuntimeOverloadProxy::optional
            )
        )
        .method(
            "required",
            static_cast<ScriptRuntimeOverloadProxy& (ScriptRuntimeOverloadProxy::*)(std::string_view, ParamKindType)>(
                &ScriptRuntimeOverloadProxy::required
            ),
            static_cast<ScriptRuntimeOverloadProxy& (ScriptRuntimeOverloadProxy::*)(std::string_view,
                                                                                    ParamKindType,
                                                                                    std::string_view)>(
                &ScriptRuntimeOverloadProxy::required
            )
        )
        .method("text", &ScriptRuntimeOverloadProxy::text)
        .method("postfix", &ScriptRuntimeOverloadProxy::postfix)
        .method("option", &ScriptRuntimeOverloadProxy::option)
        .method("deoption", &ScriptRuntimeOverloadProxy::deoption)
        .method(
            "execute",
            [](jspp::InstancePayload& payload, jspp::Arguments const& arguments) -> jspp::Local<jspp::Value> {
                {
                    auto inst = payload.unwrap<ScriptRuntimeOverloadProxy>();
                    assert(inst != nullptr);
                    inst->execute(arguments);
                }
                payload.getHolder().invalidate();
                return {};
            }
        )
        .build();

decltype(Modules::kScriptCommandParamKind) Modules::kScriptCommandParamKind = //
    helper::auto_gen_enum_def<ll::command::ParamKind::Kind>("CommandParamKind");

} // namespace land::scripting