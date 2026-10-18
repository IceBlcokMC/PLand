#include "HookRegistry.h"
#include "pland/interceptor/internal/EventInterceptor.h"
#include "pland/interceptor/internal/InterceptorConfig.h"
#include "pland/interceptor/internal/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/actor/ai/goal/LayEggGoal.h"
#include "mc/world/level/BlockSource.h"

namespace land::interceptor::internal {

// https://github.com/IceBlcokMC/PLand/issues/69

LL_TYPE_INSTANCE_HOOK(
    LayEggGoalHook,
    ll::memory::HookPriority::Normal,
    ::LayEggGoal,
    &::LayEggGoal::$isValidTarget,
    bool,
    ::BlockSource&    region,
    ::BlockPos const& pos
) {
    auto& db   = PLand::getInstance().getLandRegistry();
    auto  land = db.getLandAt(pos, region.getDimensionId());
    if (!hasEnvironmentPermission<&EnvironmentPerms::allowMobGrief>(land)) {
        return false; // 如果领地内不允许实体破坏，则阻止产蛋
    }
    return origin(region, pos);
}


LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<&InterceptorConfig::Hooks::LayEggGoalHook, LayEggGoalHook>();
});

} // namespace land::interceptor::internal