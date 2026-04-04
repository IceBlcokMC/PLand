#pragma once
#include "ll/api/service/Bedrock.h"

#include "mc/_HeaderOutputPredefine.h"
#include "mc/deps/core/utility/MCRESULT.h"
#include "mc/server/commands/CommandContext.h"
#include "mc/server/commands/CommandVersion.h"
#include "mc/server/commands/MinecraftCommands.h"
#include "mc/server/commands/PlayerCommandOrigin.h"
#include "mc/world/Minecraft.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include <mc/deps/core/utility/optional_ref.h>

#include <memory>
#include <string>


namespace mc_utils {

inline void executeCommand(const std::string& cmd, Player& player) {
    auto& minecraftCommands = ll::service::getMinecraft()->mCommands;
    if (!minecraftCommands) {
        return;
    }
    CommandContext ctx = CommandContext(
        cmd,
        std::make_unique<PlayerCommandOrigin>(player.getLevel(), player.getOrCreateUniqueID()),
        static_cast<int>(CurrentCmdVersion::Latest)
    );
    minecraftCommands->executeCommand(ctx, true);
}

[[nodiscard]] inline BlockPos face2Pos(BlockPos const& sour, uchar face) {
    BlockPos dest = sour;
    switch (face) {
    case 0:
        --dest.y; // 下
        break;
    case 1:
        ++dest.y; // 上
        break;
    case 2:
        --dest.z; // 北
        break;
    case 3:
        ++dest.z; // 南
        break;
    case 4:
        --dest.x; // 西
        break;
    case 5:
        ++dest.x; // 东
        break;
    default:
        // Unknown face
        break;
    }
    return dest;
}


} // namespace mc_utils
