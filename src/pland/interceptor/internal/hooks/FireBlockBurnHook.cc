#include "HookRegistry.h"
#include "pland/interceptor/internal/EventInterceptor.h"
#include "pland/interceptor/internal/InterceptorConfig.h"
#include "pland/interceptor/internal/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/FireBlock.h"

namespace land::internal::interceptor {

// https://github.com/IceBlcokMC/PLand/issues/136

LL_TYPE_INSTANCE_HOOK(
    FireBlockBurnHook,
    ll::memory::HookPriority::Normal,
    FireBlock,
    &FireBlock::checkBurn,
    void,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    int               chance,
    ::IRandom&        random,
    int               age,
    ::BlockPos const& firePos
) {
    auto& db   = PLand::getInstance().getLandRegistry();
    auto  land = db.getLandAt(pos, region.getDimensionId());
    if (!hasEnvironmentPermission<&EnvironmentPerms::allowFireSpread>(land)) {
        return; // 如果领地内不允许火焰蔓延，则阻止蔓延
    }
    origin(region, pos, chance, random, age, firePos);
}

LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<&InterceptorConfig::Hooks::FireBlockBurnHook, FireBlockBurnHook>();
});

} // namespace land::internal::interceptor