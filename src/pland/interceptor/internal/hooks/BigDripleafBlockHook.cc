#include "HookRegistry.h"
#include "pland/interceptor/internal/EventInterceptor.h"
#include "pland/interceptor/internal/InterceptorConfig.h"
#include "pland/interceptor/internal/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/BigDripleafBlock.h"

namespace land::internal::interceptor {

// https://github.com/IceBlcokMC/PLand/issues/192
LL_TYPE_INSTANCE_HOOK(
    BigDripleafBlockHook,
    ll::memory::HookPriority::Normal,
    BigDripleafBlock,
    &BigDripleafBlock::$entityInside,
    void,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Actor&          entity
) {
    auto& registry = PLand::getInstance().getLandRegistry();
    if (auto land = registry.getLandAt(pos, region.getDimensionId())) {
        if (entity.getEntityTypeId() == ActorType::Player) {
            // 玩家触发
            auto& player = static_cast<Player&>(entity);
            if (!hasRolePermission<&RolePerms::allowTriggerDripleaf>(land, player.getUuid())) {
                return;
            }

            // 实体触发
        } else if (!hasGuestPermission<&RolePerms::allowTriggerDripleaf>(land)) {
            return;
        }
    }
    origin(region, pos, entity);
}


LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<&InterceptorConfig::Hooks::BigDripleafBlockHook, BigDripleafBlockHook>();
});

} // namespace land::internal::interceptor