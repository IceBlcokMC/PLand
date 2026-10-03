#include "HookRegistry.h"
#include "pland/PLand.h"
#include "pland/internal/interceptor/EventInterceptor.h"
#include "pland/internal/interceptor/InterceptorConfig.h"
#include "pland/internal/interceptor/helper/InterceptorHelper.h"
#include "pland/land/repo/LandRegistry.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/level/BlockSource.h"
#include "mc/world/level/WorldBlockTarget.h"
#include "mc/world/level/levelgen/feature/VegetationPatchFeature.h"

namespace land::internal::interceptor {


// 苔藓生长 (VegetationPatchFeature) 面积检查
// MC v26.32 起 _placeGroundPatch 被内联, MossGrowthBeforeEvent 点位回退到 $place 且不再暴露 patch 半径,
// 单点检查导致斑块可从禁生长领地外越界流入. 旧版行为: 取消 _placeGroundPatch 即丢弃整个
// 地面板块并跳过植被生成, 此处以 $place 返回空 optional 达到同等效果.
// 半径在 origin() 内部才随机确定, 故取 this->mHorizontalRadius.rangeMax + 1 作为保守上界.
LL_TYPE_INSTANCE_HOOK(
    VegetationPatchPlaceHook,
    ll::memory::HookPriority::Normal,
    ::VegetationPatchFeature,
    &::VegetationPatchFeature::$place,
    std::optional<BlockPos>,
    ::IFeature::PlacementContext const& context
) {
    auto& blockSource = static_cast<::WorldBlockTarget&>(context.mTarget).mBlockSource;
    auto& blockPos    = context.mPos.get();

    auto radius = this->mHorizontalRadius->rangeMax + 1;
    auto minPos = BlockPos{blockPos.x - radius, blockPos.y - 1, blockPos.z - radius};
    auto maxPos = BlockPos{blockPos.x + radius, blockPos.y + 1, blockPos.z + radius};

    auto& registry = PLand::getInstance().getLandRegistry();
    for (auto const& land : registry.getLandAt(minPos, maxPos, blockSource.getDimensionId())) {
        if (!hasEnvironmentPermission<&EnvironmentPerms::allowMossGrowth>(land)) {
            return std::nullopt; // 取消整个植被斑块生成
        }
    }
    return origin(context);
}

LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<&HookConfig::VegetationPatchPlaceHook, VegetationPatchPlaceHook>();
});

} // namespace land::internal::interceptor