#include "HookRegistry.h"
#include "pland/interceptor/internal/EventInterceptor.h"
#include "pland/interceptor/internal/InterceptorConfig.h"
#include "pland/interceptor/internal/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/scripting/event_handlers/ScriptBlockGameplayHandler.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/events/BlockTryPlaceByPlayerEvent.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/VanillaBlockTypeIds.h"
#include "mc/world/level/block/block_events/BlockPlayerInteractEvent.h"
#include <mc/deps/core/string/HashedString.h>


namespace land::internal::interceptor {


// https://github.com/IceBlcokMC/PLand/issues/251
LL_TYPE_INSTANCE_HOOK(
    ScaffoldingBlockPlaceHook,
    ll::memory::HookPriority::High,
    ::ScriptBlockGameplayHandler,
    &::ScriptBlockGameplayHandler::$handleEvent,
    GameplayHandlerResult<CoordinatorResult>,
    ::BlockTryPlaceByPlayerEvent const& eventData
) {
    TRACE_THIS_EVENT(ScaffoldingBlockPlaceHook);

    TRACE_LOG("block={}, pos={}", eventData.mPermutationToPlace.getTypeName(), eventData.mPos.get());

    // 用引擎的哈希方块名比较(整数比较)
    HashedString const& blockName = eventData.mPermutationToPlace.getBlockType().mNameInfo->mFullName.get();
    if (blockName != VanillaBlockTypeIds::Scaffolding()) {
        TRACE_LOG("block is not a ScaffoldingBlock, skip");
        return origin(eventData);
    }

    auto actor = eventData.mPlayer->tryUnwrap();
    if (!actor || actor->getEntityTypeId() != ActorType::Player) {
        TRACE_LOG("placer is not a player or player not found, skip");
        return origin(eventData);
    }

    auto& player   = static_cast<Player&>(actor.value());
    auto& registry = PLand::getInstance().getLandRegistry();

    auto land = registry.getLandAt(eventData.mPos.get(), player.getDimensionId());
    if (!hasRolePermission<&RolePerms::allowPlace>(land, player.getUuid())) {
        return {.handler_result = HandlerResult::BypassListeners, .return_value = CoordinatorResult::Cancel};
    }
    return origin(eventData);
}


LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<&HookConfig::ScaffoldingBlockHook, ScaffoldingBlockPlaceHook>();
});

} // namespace land::internal::interceptor
