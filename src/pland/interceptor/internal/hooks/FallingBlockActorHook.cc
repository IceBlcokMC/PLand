#include "HookRegistry.h"
#include "pland/interceptor/internal/EventInterceptor.h"
#include "pland/interceptor/internal/InterceptorConfig.h"
#include "pland/interceptor/internal/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/legacy/ActorUniqueID.h"
#include "mc/world/actor/item/FallingBlockActor.h"

#include <absl/container/flat_hash_map.h>

namespace land::internal::interceptor {


// Fix [#242](https://github.com/IceBlcokMC/PLand/issues/242)
// 仅拦截"自领地外坠入领地"的下落实体, 领地内起始的下落保持原版行为

namespace {
// key: ActorUniqueID::rawID -> value: 实体创建时的起始坐标
absl::flat_hash_map<int64_t, BlockPos> sFallingBlockStartCache;
} // namespace

LL_TYPE_INSTANCE_HOOK(
    FallingBlockActorTickHook,
    ll::memory::HookPriority::Normal,
    FallingBlockActor,
    &FallingBlockActor::$normalTick,
    void
) {
    auto blockPos = BlockPos{this->getPosition()};
    auto uid      = this->getOrCreateUniqueID().rawID;

    // 记录起始坐标
    auto [iter, inserted] = sFallingBlockStartCache.try_emplace(uid, blockPos);
    auto const& startPos  = iter->second;

    auto& registry = PLand::getInstance().getLandRegistry();
    // 实体处于领地内时判定下落来源
    if (auto land = registry.getLandAt(blockPos, this->getDimensionId());
        land && !land->getAABB().isAboveLand(blockPos)) {
        auto const& aabb = land->getAABB();
        // 下落是否起始于同一领地
        bool startedInside = aabb.hasPos(startPos, land->is3D()) && !aabb.isAboveLand(startPos);
        if (!startedInside && !hasEnvironmentPermission<&EnvironmentPerms::allowBlockFall>(land)) {
            sFallingBlockStartCache.erase(uid);
            this->breakBlock();
            return;
        }
    }
    origin();
}
LL_TYPE_INSTANCE_HOOK(FallingBlockActorRemoveHook, ll::memory::HookPriority::Normal, ::Actor, &::Actor::$remove, void) {
    if (this->getEntityTypeId() == ActorType::FallingBlock) {
        sFallingBlockStartCache.erase(this->getOrCreateUniqueID().rawID);
    }
    origin();
}


LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<
        &InterceptorConfig::Hooks::FallingBlockActorTickHook,
        FallingBlockActorTickHook,
        FallingBlockActorRemoveHook>([]() { sFallingBlockStartCache.clear(); });
});

} // namespace land::internal::interceptor