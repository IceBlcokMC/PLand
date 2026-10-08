#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "mc/world/level/Level.h"
#include <mc/world/actor/player/Player.h>

#include "ll/api/service/Bedrock.h"

namespace land::scripting {

decltype(Modules::kScriptPlayer) Modules::kScriptPlayer = //
    jspp::binding::defClass<Player>("Player")
        .ctor(nullptr)
        .func(
            "tryGet",
            [](std::string const& realName) -> Player* {
                if (auto level = ll::service::getLevel()) {
                    return level->getPlayer(realName);
                }
                return nullptr;
            }
        )
        .prop_readonly("localeCode", &Player::getLocaleCode)
        .prop_readonly("realName", &Player::getRealName)
        .prop_readonly("xuid", &Player::getXuid)
        .prop_readonly("uuid", [](Player* player) { return player->getUuid().asString(); })
        .build();


} // namespace land::scripting