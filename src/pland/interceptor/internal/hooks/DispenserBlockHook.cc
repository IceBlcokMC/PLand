#include "HookRegistry.h"
#include "pland/interceptor/internal/EventInterceptor.h"
#include "pland/interceptor/internal/InterceptorConfig.h"
#include "pland/interceptor/internal/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/item/BucketItem.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/DispenserBlock.h"
#include "mc/world/level/block/VanillaStates.h"
#include "mc/world/level/block/actor/DispenserBlockActor.h"

namespace land::internal::interceptor {


// Fix [#231](https://github.com/IceBlcokMC/PLand/issues/231)

namespace {
thread_local bool tBlockCurrentDispense = false; // dispenseFrom -> BucketItem::$dispense 同步调用栈内传递拦截标记

struct BlockDispenseGuard {
    BlockDispenseGuard() { tBlockCurrentDispense = true; }
    ~BlockDispenseGuard() { tBlockCurrentDispense = false; }
};

} // namespace

LL_TYPE_INSTANCE_HOOK(
    DispenserDispenseFromHook,
    ll::memory::HookPriority::Normal,
    DispenserBlock,
    &DispenserBlock::$dispenseFrom,
    void,
    ::BlockSource&    region,
    ::BlockPos const& pos
) {
    auto& registry = PLand::getInstance().getLandRegistry();
    auto  dimid    = region.getDimensionId();

    // Do not call getDispensePosition here. Its Vec3-by-value ABI corrupts this
    // hook's stack frame on BDS 26.51. Read the state directly and find the
    // adjacent block the dispenser is facing instead.
    auto facing = region.getBlock(pos).getState<int>(VanillaStates::FacingDirection());
    if (!facing || *facing < 0 || *facing > 5) {
        origin(region, pos);
        return;
    }

    auto targetPos = pos;
    switch (*facing) {
    case 0:
        --targetPos.y;
        break; // down
    case 1:
        ++targetPos.y;
        break; // up
    case 2:
        --targetPos.z;
        break; // north
    case 3:
        ++targetPos.z;
        break; // south
    case 4:
        --targetPos.x;
        break; // west
    case 5:
        ++targetPos.x;
        break; // east
    default:
        break;
    }

    if (
        auto targetLand = registry.getLandAt(targetPos, dimid);
        targetLand && !hasEnvironmentPermission<&EnvironmentPerms::allowLiquidFlow>(targetLand) // 禁止液体流动
        && targetLand->getAABB().isOnOuterBoundary(pos)                                         // 发射器紧贴领地外
        && targetLand->getAABB().isOnInnerBoundary(targetPos) // 发射目标位于领地内边界
    ) {
        BlockDispenseGuard guard;
        origin(region, pos); // 具体是否拦截由 BucketItem::$dispense 按选中的物品判定
        return;
    }
    origin(region, pos);
}
LL_TYPE_INSTANCE_HOOK(
    DispenserGetItemHook,
    ll::memory::HookPriority::Normal,
    DispenserBlockActor,
    &DispenserBlockActor::$getItem,
    ::ItemStack const&,
    int slot
) {
    auto& itemStack = origin(slot);
    if (!itemStack.isNull() && tBlockCurrentDispense) {
        if (auto item = itemStack.mItem.get(); item && item->isBucket()) {
            return ItemStack::EMPTY_ITEM();
        }
    }
    return itemStack;
    // int getRandomSlot(Random& random); // v26.20
    // if (slot >= 0 && tBlockCurrentDispense && isLiquidBucketItem(this->getItem(slot))) {
    //     return -1; // 随机选中的是液体桶: 以"无可用槽位"语义取消本次发射
    // }
    // return slot;
}


LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor
        .registerHookIf<&HookConfig::DispenserLiquidDispenseHook, DispenserDispenseFromHook, DispenserGetItemHook>();
});

} // namespace land::internal::interceptor