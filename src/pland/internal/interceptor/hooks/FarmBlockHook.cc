#include "HookRegistry.h"
#include "pland/internal/interceptor/EventInterceptor.h"
#include "pland/internal/interceptor/InterceptorConfig.h"
#include "pland/internal/interceptor/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/FarmBlock.h"

namespace land::internal::interceptor {


// https://github.com/IceBlcokMC/PLand/issues/191
LL_TYPE_INSTANCE_HOOK(
    FarmChangeEventHook,
    ll::memory::HookPriority::Normal,
    FarmBlock,
    &FarmBlock::$transformOnFall,
    void,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Actor*          actor,
    float             fallDistance
) {
    auto& registry = PLand::getInstance().getLandRegistry();
    auto  land     = registry.getLandAt(pos, region.getDimensionId());
    if (!hasEnvironmentPermission<&EnvironmentPerms::allowFarmDecay>(land)) {
        return;
    }

    // Falling entities still trigger farmland decay; only players need role checks.
    if (actor && actor->getEntityTypeId() == ActorType::Player) {
        auto& player = static_cast<Player&>(*actor);
        if (!hasRolePermission<&RolePerms::allowDestroy>(land, player.getUuid())) {
            return;
        }
    }

    origin(region, pos, actor, fallDistance);
}


LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<&InterceptorConfig::Hooks::FarmChangeEventHook, FarmChangeEventHook>();
});

} // namespace land::internal::interceptor