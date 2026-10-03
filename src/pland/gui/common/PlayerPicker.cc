#include "PlayerPicker.h"
#include "pland/gui/utils/BackUtils.h"
#include "pland/utils/FeedbackUtils.h"

#include "ll/api/form/CustomForm.h"
#include "ll/api/form/SimpleForm.h"
#include <ll/api/service/PlayerInfo.h>

#include "mc/deps/ecs/WeakEntityRef.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"


namespace land::gui {

using ll::form::CustomForm;
using ll::form::SimpleForm;

void PlayerPickerRouter::sendTo(
    Player&           player,
    callback_t const& callback,
    back_t const&     backTo,
    bool              includeSimulatedPlayers
) {
    auto localeCode = player.getLocaleCode();

    auto f = SimpleForm{};
    f.setTitle("[PLand] 玩家选择模式"_trl(localeCode));

    if (backTo) {
        back_utils::injectBackButton(f, backTo);
    }

    f.appendButton(
        "在线玩家"_trl(localeCode),
        "textures/ui/player_online_icon.png",
        "path",
        [callback, backTo, includeSimulatedPlayers](Player& player) {
            OnlinePlayerPicker::sendTo(player, callback, backTo, includeSimulatedPlayers);
        }
    );
    f.appendButton(
        "离线玩家"_trl(localeCode),
        "textures/ui/player_offline_icon.png",
        "path",
        [callback](Player& player) { OfflinePlayerPicker::sendTo(player, callback); }
    );

    f.sendTo(player);
}

void OnlinePlayerPicker::sendTo(
    Player&                               player,
    PlayerPickerRouter::callback_t const& callback,
    PlayerPickerRouter::back_t            backTo,
    bool                                  includeSimulatedPlayers
) {
    SimpleForm f{"[PLand] | 玩家选择器"_trl(player.getLocaleCode())};

    if (backTo) {
        back_utils::injectBackButton(f, std::move(backTo));
    }

    player.getLevel().forEachPlayer([&f, &callback, includeSimulatedPlayers](Player& target) {
        if (target.isSimulatedPlayer() && !includeSimulatedPlayers) {
            return true; // skip
        }
        f.appendButton(target.getRealName(), [weak = target.getEntityContext().getWeakRef(), callback](Player& player) {
            if (auto target = weak.tryUnwrap<Mob>(); target && target->getEntityTypeId() == ActorType::Player) {
                callback(player, static_cast<Player*>(target.as_ptr()));
            }
        });
        return true;
    });

    f.sendTo(player);
}

void OfflinePlayerPicker::sendTo(
    Player&                               player,
    PlayerPickerRouter::callback_t const& callback,
    const std::string&                    defaultVal,
    const std::string&                    errorText,
    int                                   retryCount,
    int                                   maxRetries
) {
    auto localeCode = player.getLocaleCode();

    if (retryCount > maxRetries) {
        feedback_utils::sendErrorText(player, "操作失败!"_trl(localeCode));
        return;
    }

    CustomForm fm("[PLand] | 离线玩家"_trl(localeCode));
    fm.appendInput("playerName", "请输入离线玩家名称"_trl(localeCode), "玩家名称", defaultVal);

    if (!errorText.empty()) {
        fm.appendLabel(errorText);
    }

    fm.sendTo(
        player,
        [callback,
         retryCount,
         maxRetries](Player& self, ll::form::CustomFormResult const& res, ll::form::FormCancelReason) {
            if (!res) {
                return;
            }
            auto localeCode = self.getLocaleCode();

            auto playerName = std::get<std::string>(res->at("playerName"));
            if (playerName.empty()) {
                sendTo(self, callback, playerName, "玩家名称不能为空!"_trl(localeCode), retryCount + 1, maxRetries);
                return;
            }

            auto playerInfo = ll::service::PlayerInfo::getInstance().fromName(playerName);
            if (!playerInfo) {
                sendTo(
                    self,
                    callback,
                    playerName,
                    "未找到该玩家信息，请检查名称是否正确!"_trl(localeCode),
                    retryCount + 1,
                    maxRetries
                );
                return;
            }
            callback(self, playerInfo);
        }
    );
}


} // namespace land::gui
