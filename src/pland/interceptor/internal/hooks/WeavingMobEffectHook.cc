#include "HookRegistry.h"
#include "pland/internal/interceptor/EventInterceptor.h"
#include "pland/internal/interceptor/InterceptorConfig.h"
#include "pland/internal/interceptor/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/effect/WeavingMobEffect.h"

namespace land::internal::interceptor {

// https://github.com/IceBlcokMC/PLand/issues/59

LL_TYPE_INSTANCE_HOOK(
    WeavingMobEffectHook,
    ll::memory::HookPriority::Normal,
    WeavingMobEffect,
    &WeavingMobEffect::$onActorDied,
    void,
    ::Actor& actor,
    int      amplifier
) {
    // Wiki: 当游戏规则mobGriefing为true时，拥有盘丝的生物死亡后会在以自身为中心3×3×3的范围内尝试生成2-3个蜘蛛网
    auto& pos      = actor.getPosition();
    auto& registry = PLand::getInstance().getLandRegistry();
    auto  land     = registry.getLandAt(pos, actor.getDimensionId());
    if (!hasEnvironmentPermission<&EnvironmentPerms::allowMobGrief>(land)) {
        return;
    }
    origin(actor, amplifier);
}

LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<&InterceptorConfig::Hooks::WeavingMobEffectHook, WeavingMobEffectHook>();
});

} // namespace land::internal::interceptor