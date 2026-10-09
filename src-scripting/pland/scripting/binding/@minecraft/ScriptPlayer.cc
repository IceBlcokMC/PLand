#include "pland/scripting/binding/Helper.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "mc/network/packet/ToastRequestPacket.h"
#include "mc/world/level/Level.h"
#include <mc/network/packet/SetTitlePacket.h>
#include <mc/world/actor/player/Player.h>

#include "ll/api/service/Bedrock.h"

namespace land::scripting {

decltype(Modules::kScriptPlayer) Modules::kScriptPlayer = //
    jspp::binding::defClass<Player>("Player")
        .ctor(nullptr)
        .func(
            "tryGet",
            [](std::string const& name_or_uid) -> Player* {
                if (auto level = ll::service::getLevel()) {
                    if (mce::UUID::canParse(name_or_uid)) {
                        return level->getPlayer(mce::UUID::fromString(name_or_uid));
                    } else {
                        return level->getPlayer(name_or_uid); // name
                    }
                }
                return nullptr;
            }
        )
        .prop_readonly("localeCode", &Player::getLocaleCode)
        .prop_readonly("realName", &Player::getRealName)
        .prop_readonly("xuid", &Player::getXuid)
        .prop_readonly("isOperator", &Player::isOperator)
        .prop_readonly("uuid", [](Player* player) { return player->getUuid().asString(); })
        .method(
            "sendMessage",
            [](jspp::InstancePayload& payload, jspp::Arguments const& args) -> jspp::Local<jspp::Value> {
                auto player = payload.unwrap<Player>();
                player->sendMessage(helper::format(args));
                return {};
            }
        )
        .method(
            "sendToast",
            [](Player& player, std::string const& title, std::string const& content) {
                auto pkt     = ToastRequestPacket{};
                pkt.mTitle   = title;
                pkt.mContent = content;
                pkt.sendTo(player);
            }
        )
        .build();


} // namespace land::scripting