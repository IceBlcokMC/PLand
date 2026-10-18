#include "HookRegistry.h"
#include "pland/interceptor/internal/EventInterceptor.h"
#include "pland/interceptor/internal/InterceptorConfig.h"
#include "pland/interceptor/internal/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/actor/global/LightningBolt.h"

namespace land::interceptor::internal {

// https://github.com/IceBlcokMC/PLand/issues/167

LL_TYPE_INSTANCE_HOOK(
    LightningBoltHook,
    ll::memory::HookPriority::Normal,
    LightningBolt,
    &LightningBolt::$normalTick,
    void
) {
    auto& registry = PLand::getInstance().getLandRegistry();
    if (auto land = registry.getLandAt(this->getPosition(), this->getDimensionId())) {
        if (!hasEnvironmentPermission<&EnvironmentPerms::allowLightningBolt>(land)) {
            this->remove(); // 必须标记移除，否则闪电实体不会被移除且会一直tick
            return;         // 不允许闪电，拦截 tick
        }
    }
    origin();
}

LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<&InterceptorConfig::Hooks::LightningBoltHook, LightningBoltHook>();
});

} // namespace land::interceptor::internal