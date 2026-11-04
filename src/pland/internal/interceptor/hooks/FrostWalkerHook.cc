#include "HookRegistry.h"
#include "pland/PLand.h"
#include "pland/internal/interceptor/EventInterceptor.h"
#include "pland/internal/interceptor/InterceptorConfig.h"
#include "pland/internal/interceptor/helper/InterceptorHelper.h"

#include <ll/api/base/ScopedValue.h>
#include <ll/api/memory/Hook.h>

#include "mc/world/actor/Mob.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockChangeContext.h"
#include <mc/deps/core/string/HashedString.h>
#include <mc/entity/components/ActorOwnerComponent.h>
#include <mc/world/level/block/VanillaBlockTypeIds.h>


namespace land::internal::interceptor {

// https://github.com/IceBlcokMC/PLand/issues/250
// 由于冰霜行者范围计算在 frostWalk 内部，采用2阶段 Hook 拦截
//  v26.60 起，Mob::frostWalk 被优化

namespace {

thread_local Actor* tFrostWalker = nullptr;

using FrostWalkerSetBlockFn = bool (BlockSource::*)(
    BlockPos const&,
    Block const&,
    int,
    std::shared_ptr<BlockActor>,
    ActorBlockSyncMessage const*,
    BlockChangeContext const&
);

using namespace ll::memory_literals;

LL_AUTO_INSTANCE_HOOK(
    FrostWalkerTickHook,
    HookPriority::Normal,
    // "FrostWalkSystem::tickSystem"_sym,
    "41 57 41 56 41 55 41 54 56 57 55 53 48 81 EC ?? ?? ?? ?? 0F 29 BC 24 ?? ?? ?? ?? 0F 29 B4 24 ?? ?? ?? ?? 48 8B 31"_sig,
    void,
    StrictEntityContext const& context,
    ActorOwnerComponent const& actorOwnerComponent
) {
    ll::ScopedValue scope{tFrostWalker, actorOwnerComponent.mActor.get()};
    origin(context, actorOwnerComponent);
}

} // namespace


LL_TYPE_INSTANCE_HOOK(
    FrostWalkerSetBlockHook,
    ll::memory::HookPriority::Normal,
    BlockSource,
    static_cast<FrostWalkerSetBlockFn>(&BlockSource::setBlock),
    bool,
    BlockPos const&              pos,
    Block const&                 block,
    int                          updateFlags,
    std::shared_ptr<BlockActor>  blockActor,
    ActorBlockSyncMessage const* syncMsg,
    BlockChangeContext const&    changeSourceContext
) {
    if (tFrostWalker && block.getBlockType().mNameInfo->mFullName.get() == VanillaBlockTypeIds::FrostedIce()) {
        auto& registry = PLand::getInstance().getLandRegistry();
        auto  land     = registry.getLandAt(pos, this->getDimensionId());

        if (tFrostWalker->getEntityTypeId() == ActorType::Player) {
            auto& player = static_cast<Player&>(*tFrostWalker);
            if (!hasRolePermission<&RolePerms::allowFrostWalker>(land, player.getUuid())) {
                return false;
            }
        } else if (!hasGuestPermission<&RolePerms::allowFrostWalker>(land)) {
            return false;
        }
    }
    return origin(pos, block, updateFlags, std::move(blockActor), syncMsg, changeSourceContext);
}

LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<&HookConfig::FrostWalkerHook, FrostWalkerTickHook, FrostWalkerSetBlockHook>();
});

} // namespace land::internal::interceptor