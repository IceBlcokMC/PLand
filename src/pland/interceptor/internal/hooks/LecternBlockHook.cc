#include "HookRegistry.h"
#include "pland/interceptor/internal/EventInterceptor.h"
#include "pland/interceptor/internal/InterceptorConfig.h"
#include "pland/interceptor/internal/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/level/block/LecternBlock.h"
#include "mc/world/level/block/block_events/BlockPlayerInteractEvent.h"

namespace land::interceptor::internal {

// https://github.com/IceBlcokMC/PLand/issues/143

LL_TYPE_INSTANCE_HOOK(
    LecternBlockUseHook,
    ll::memory::HookPriority::Normal,
    LecternBlock,
    &LecternBlock::use,
    void,
    BlockEvents::BlockPlayerInteractEvent& event
) {
    auto& player = event.mPlayer;
    auto& pos    = event.mPos;

    auto& registry = PLand::getInstance().getLandRegistry();
    auto  land     = registry.getLandAt(pos, player.getDimensionId());
    if (!hasRolePermission<&RolePerms::useLectern>(land, player.getUuid())) {
        return; // 拦截阅读/放置书本
    }
    origin(event);
}
LL_TYPE_INSTANCE_HOOK(
    LecternBlockDropBookHook,
    ll::memory::HookPriority::Normal,
    LecternBlock,
    &LecternBlock::$attack,
    bool,
    Player*         player,
    BlockPos const& pos
) {
    if (!player) {
        return origin(player, pos);
    }

    auto& registry = PLand::getInstance().getLandRegistry();
    auto  land     = registry.getLandAt(pos, player->getDimensionId());
    if (!hasRolePermission<&RolePerms::useLectern>(land, player->getUuid())) {
        return false; // 拦截取下书本
    }
    return origin(player, pos);
}


LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<&InterceptorConfig::Hooks::LecternBlockUseHook, LecternBlockUseHook>();
    interceptor.registerHookIf<&InterceptorConfig::Hooks::LecternBlockDropBookHook, LecternBlockDropBookHook>();
});

} // namespace land::interceptor::internal