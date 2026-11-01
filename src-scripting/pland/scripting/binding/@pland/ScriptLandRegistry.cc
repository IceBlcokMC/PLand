#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "pland/PLand.h"
#include "pland/land/Land.h"
#include "pland/land/repo/LandRegistry.h"


namespace land::scripting {

decltype(Modules::kScriptLandRegistry) Modules::kScriptLandRegistry =
    jspp::binding::defClass<void>("LandRegistry")
        .instanceProvider([]() -> auto& { return PLand::getInstance().getLandRegistry(); })
        .func("isAdmin", &LandRegistry::isOperator)
        .func("addAdmin", &LandRegistry::addOperator)
        .func("removeAdmin", &LandRegistry::removeOperator)
        .func("getAdmins", &LandRegistry::getOperators)
        .func("hasLand", &LandRegistry::hasLand)
        .func("getLandById", &LandRegistry::getLand)
        .func(
            "getLandByIds",
            static_cast<std::vector<std::shared_ptr<Land>> (LandRegistry::*)(std::vector<LandID> const&) const>(
                &LandRegistry::getLands
            )
        )
        .func(
            "getLandsByOwner",
            static_cast<std::vector<std::shared_ptr<Land>> (LandRegistry::*)(mce::UUID const&, bool) const>(
                &LandRegistry::getLands
            )
        )
        .func(
            "getLandsByOwnerInDimension",
            static_cast<std::vector<std::shared_ptr<Land>> (LandRegistry::*)(mce::UUID const&, LandDimid) const>(
                &LandRegistry::getLands
            )
        )
        .func("getOwnerLandCounts", &LandRegistry::getOwnerLandCounts)
        .func("getEffectiveRole", &LandRegistry::getEffectiveRole)
        .func(
            "getLandAtPosition",
            static_cast<std::shared_ptr<Land> (LandRegistry::*)(BlockPos const&, LandDimid) const>(
                &LandRegistry::getLandAt
            )
        )
        .func(
            "getLandsInRadius",
            static_cast<std::unordered_set<std::shared_ptr<Land>> (LandRegistry::*)(BlockPos const&, int, LandDimid)
                            const>(&LandRegistry::getLandAt)
        )
        .func(
            "getLandsIntersectingBounds",
            static_cast<
                std::unordered_set<std::shared_ptr<Land>> (LandRegistry::*)(BlockPos const&, BlockPos const&, LandDimid)
                    const>(&LandRegistry::getLandAt)
        )
        .build();

} // namespace land::scripting