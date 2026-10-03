#include "HookRegistry.h"
#include "pland/internal/interceptor/EventInterceptor.h"
#include "pland/internal/interceptor/InterceptorConfig.h"
#include "pland/internal/interceptor/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/effect/OozingMobEffect.h"

namespace land::internal::interceptor {

// https://github.com/IceBlcokMC/PLand/issues/59

LL_TYPE_INSTANCE_HOOK(
    OozingMobEffectHook,
    ll::memory::HookPriority::Normal,
    OozingMobEffect,
    &OozingMobEffect::$onActorDied,
    void,
    ::Actor& actor,
    int      amplifier
) {
    // Wiki: 此效果的生物死亡时，会尝试在死亡处生成2只中型史莱姆
    auto& pos      = actor.getPosition();
    auto& registry = PLand::getInstance().getLandRegistry();
    auto  land     = registry.getLandAt(pos, actor.getDimensionId());
    if (!hasEnvironmentPermission<&EnvironmentPerms::allowMonsterSpawn>(land)) {
        return;
    }
    origin(actor, amplifier);
}

LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<&InterceptorConfig::Hooks::OozingMobEffectHook, OozingMobEffectHook>();
});

} // namespace land::internal::interceptor