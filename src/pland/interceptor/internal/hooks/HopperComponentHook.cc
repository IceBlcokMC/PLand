#include "HookRegistry.h"
#include "pland/interceptor/internal/EventInterceptor.h"
#include "pland/interceptor/internal/InterceptorConfig.h"
#include "pland/interceptor/internal/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/entity/components_json_legacy/HopperComponent.h"

namespace land::interceptor::internal {

// https://github.com/IceBlcokMC/PLand/issues/55

LL_TYPE_INSTANCE_HOOK(
    HopperComponentPullInItemsHook,
    ll::memory::HookPriority::Normal,
    HopperComponent,
    &HopperComponent::pullInItems,
    bool,
    ::Actor& owner // 拥有此组件的 Actor
) {
    if (owner.getEntityTypeId() != ActorType::MinecartHopper) {
        return origin(owner);
    }

    auto& registry = PLand::getInstance().getLandRegistry();
    auto  land     = registry.getLandAt(owner.getPosition(), owner.getDimensionId());
    if (!hasEnvironmentPermission<&EnvironmentPerms::allowMinecartHopperPullItems>(land)) {
        return false;
    }
    return origin(owner);
}


LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor
        .registerHookIf<&InterceptorConfig::Hooks::HopperComponentPullInItemsHook, HopperComponentPullInItemsHook>();
});

} // namespace land::interceptor::internal