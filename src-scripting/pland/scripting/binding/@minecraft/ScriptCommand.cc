#include "pland/scripting/binding/Helper.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOutput.h>

namespace land::scripting {

decltype(Modules::kScriptCommandOrigin) Modules::kScriptCommandOrigin = //
    jspp::binding::defClass<CommandOrigin>("CommandOrigin")
        .ctor(nullptr)
        .prop_readonly("originType", &CommandOrigin::getOriginType)
        .prop_readonly("blockPosition", &CommandOrigin::getBlockPosition)
        .prop_readonly("worldPosition", &CommandOrigin::getWorldPosition)
        .prop_readonly(
            "player",
            [](CommandOrigin const& origin) -> Player* {
                if (origin.getOriginType() == CommandOriginType::Player) {
                    if (auto entity = origin.getEntity(); entity && entity->isPlayer()) {
                        return static_cast<Player*>(entity);
                    }
                }
                return nullptr;
            },
            jspp::binding::ReturnValuePolicy::kReferencePersistent
        )
        .build();

decltype(Modules::kScriptCommandOriginType) Modules::kScriptCommandOriginType = //
    helper::auto_gen_enum_def<CommandOriginType>("CommandOriginType");

decltype(Modules::kScriptCommandOutput) Modules::kScriptCommandOutput = //
    jspp::binding::defClass<CommandOutput>("CommandOutput")
        .ctor(nullptr)
        .method(
            "success",
            [](jspp::InstancePayload& payload, jspp::Arguments const& args) -> jspp::Local<jspp::Value> {
                auto self = payload.unwrap<CommandOutput>();
                auto v    = helper::format(args);
                self->success("{}", v);
                return {};
            }
        )
        .method(
            "error",
            [](jspp::InstancePayload& payload, jspp::Arguments const& args) -> jspp::Local<jspp::Value> {
                auto self = payload.unwrap<CommandOutput>();
                auto v    = helper::format(args);
                self->error("{}", v);
                return {};
            }
        )
        .build();


} // namespace land::scripting