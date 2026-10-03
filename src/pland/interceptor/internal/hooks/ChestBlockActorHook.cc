#include "HookRegistry.h"
#include "pland/interceptor/internal/EventInterceptor.h"
#include "pland/interceptor/internal/InterceptorConfig.h"
#include "pland/interceptor/internal/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/actor/ActorType.h"
#include "mc/world/level/block/actor/ChestBlockActor.h"

namespace land::internal::interceptor {

// https://github.com/IceBlcokMC/PLand/issues/158

LL_TYPE_INSTANCE_HOOK(
    ChestBlockActorOpenHook,
    ll::memory::HookPriority::Normal,
    ChestBlockActor,
    &ChestBlockActor::$startOpen,
    void,
    ::Actor& actor
) {
    if (actor.getEntityTypeId() == ActorType::Player) {
        origin(actor);
        return;
    }
    auto& db   = PLand::getInstance().getLandRegistry();
    auto  land = db.getLandAt(this->mPosition, actor.getDimensionId());
    if (!hasGuestPermission<&RolePerms::useContainer>(land)) {
        return; // 访客权限不允许，拦截铜傀儡开箱子
    }
    origin(actor);
}

LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<&InterceptorConfig::Hooks::ChestBlockActorOpenHook, ChestBlockActorOpenHook>();
});

} // namespace land::internal::interceptor