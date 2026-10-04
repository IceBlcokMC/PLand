#include "HookRegistry.h"
#include "pland/interceptor/internal/EventInterceptor.h"
#include "pland/interceptor/internal/InterceptorConfig.h"
#include "pland/interceptor/internal/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/entity/components/ActorOwnerComponent.h"
#include "mc/entity/components/DealKineticDamageComponent.h"
#include "mc/entity/systems/DealKineticDamageSystem.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/phys/AABB.h"

namespace land::interceptor::internal {

// Fix [#231](https://github.com/IceBlcokMC/PLand/issues/231)
// TODO: 精确的命中查询 (HitDetection::MeleeTargeting::getHitResults) 为 MCNAPI 符号
// https://github.com/LiteLDev/mcapi-requests/issues/236
// https://github.com/LiteLDev/mcapi-requests/issues/237
// In BDS 26.51 the EnTT query type is no longer an ABI parameter. Hook the
// generated two-argument function directly instead of relying on a mangled symbol.
LL_STATIC_HOOK(
    KineticDamageSystemHook,
    ll::memory::HookPriority::Normal,
    &DealKineticDamageSystem::tryApplyDamageOrEffects,
    void,
    ::ActorOwnerComponent&        owner,
    ::DealKineticDamageComponent& component
) {
    auto* attacker = owner.mActor.get();
    if (attacker && attacker->getEntityTypeId() == ActorType::Player) {
        auto& player = static_cast<Player&>(*attacker);
        auto& uuid   = player.getUuid();

        // 以攻击者为中心的宽松包围盒 (矛最大触及 7.5 + 眼高 + 命中边际), 判定冲刺触及范围
        auto center = attacker->getPosition();
        AABB box{
            center - Vec3{12, 12, 12},
            center + Vec3{12, 12, 12}
        };
        auto& region = attacker->getDimensionBlockSource();
        for (auto& handle : region.fetchEntities(attacker, box, false, false)) {
            auto* entity = handle.get();
            if (entity && !hasPlayerDamagePermission(*entity, uuid)) {
                return; // 触及范围内存在受保护实体: 本次 sweep 整体不执行
            }
        }
    }
    origin(owner, component);
}


LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<&HookConfig::KineticDamageHook, KineticDamageSystemHook>();
});

} // namespace land::interceptor::internal