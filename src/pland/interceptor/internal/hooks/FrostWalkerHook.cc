#include "HookRegistry.h"
#include "pland/PLand.h"
#include "pland/interceptor/internal/EventInterceptor.h"
#include "pland/interceptor/internal/InterceptorConfig.h"
#include "pland/interceptor/internal/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/actor/Mob.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockChangeContext.h"
#include <mc/deps/core/string/HashedString.h>
#include <mc/world/level/block/VanillaBlockTypeIds.h>

namespace land::interceptor::internal {

// https://github.com/IceBlcokMC/PLand/issues/250
// 由于冰霜行者范围计算在 frostWalk 内部，采用2阶段 Hook 拦截

namespace {

thread_local Mob* tFrostWalker = nullptr;

struct FrostWalkerGuard {
    Mob* previous;

    explicit FrostWalkerGuard(Mob& mob) : previous(tFrostWalker) { tFrostWalker = &mob; }
    ~FrostWalkerGuard() { tFrostWalker = previous; }
};

using FrostWalkerSetBlockFn = bool (BlockSource::*)(
    BlockPos const&,
    Block const&,
    int,
    std::shared_ptr<BlockActor>,
    ActorBlockSyncMessage const*,
    BlockChangeContext const&
);

} // namespace

LL_TYPE_INSTANCE_HOOK(FrostWalkerHook, ll::memory::HookPriority::Normal, Mob, &Mob::frostWalk, void) {
    FrostWalkerGuard guard{*this};
    origin();
}

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
    interceptor.registerHookIf<&HookConfig::FrostWalkerHook, FrostWalkerHook, FrostWalkerSetBlockHook>();
});

} // namespace land::interceptor::internal