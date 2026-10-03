#include "HookRegistry.h"
#include "pland/interceptor/internal/EventInterceptor.h"
#include "pland/interceptor/internal/InterceptorConfig.h"
#include "pland/interceptor/internal/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/actor/FishingHook.h"
#include "mc/world/actor/player/Player.h"

namespace land::internal::interceptor {

// https://github.com/IceBlcokMC/PLand/issues/56

LL_TYPE_INSTANCE_HOOK(
    FishingHookHitHook,
    ll::memory::HookPriority::Normal,
    ::FishingHook,
    &::FishingHook::_pullCloser,
    void,
    Actor& inEntity,
    float  inSpeed
) {
    // 获取钓鱼钩的位置和维度
    auto& hookActor = *this;
    auto* player    = hookActor.getPlayerOwner();
    if (!player) {
        origin(inEntity, inSpeed);
        return;
    }

    auto& pos   = hookActor.getPosition();
    auto  dimId = hookActor.getDimensionId();

    auto& db   = PLand::getInstance().getLandRegistry();
    auto  land = db.getLandAt(pos, dimId);
    if (!hasRolePermission<&RolePerms::allowFishingRodAndHook>(land, player->getUuid())) {
        return;
    }

    origin(inEntity, inSpeed);
}

LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<&InterceptorConfig::Hooks::FishingHookHitHook, FishingHookHitHook>();
});

} // namespace land::internal::interceptor