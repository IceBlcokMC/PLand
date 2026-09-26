#pragma once
#include "pland/PLand.h"
#include "pland/land/Land.h"
#include "pland/land/repo/LandRegistry.h"
#include "pland/utils/FeedbackUtils.h"

#include "mc/world/actor/player/Player.h"

namespace land::permission_utils {

inline bool checkLandManagement(Player& player, Land const& land) {
    auto const& uuid = player.getUuid();
    if (land.isOwner(uuid) || PLand::getInstance().getLandRegistry().isOperator(uuid)) {
        return true;
    }
    feedback_utils::sendErrorText(
        player,
        land.isOwnerless() ? "无主领地仅允许领地管理员管理"_trl(player.getLocaleCode())
                           : "操作失败，您不是领地主人"_trl(player.getLocaleCode())
    );
    return false;
}

} // namespace land::permission_utils
