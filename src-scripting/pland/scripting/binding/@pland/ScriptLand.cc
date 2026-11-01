#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "pland/land/Land.h"

namespace land::scripting {


decltype(Modules::kScriptLand) Modules::kScriptLand = //
    jspp::binding::defClass<Land>("Land")
        .ctor<>() // default ctor
        .ctor([](LandAABB const& aabb, int dim, int is3D, mce::UUID const& owner) {
            return Land::make(aabb, dim, is3D, owner);
        })
        .prop_readonly("id", &Land::getId)
        .prop_readonly("is3D", &Land::is3D)
        .prop_readonly("dimensionId", &Land::getDimensionId)
        .prop_readonly("isRegistered", &Land::isRegistered)
        .prop_readonly("isOwnerless", &Land::isOwnerless)
        .prop_readonly("isPlayerOwned", &Land::isPlayerOwned)
        .prop_readonly("isSystemOwned", &Land::isSystemOwned)
        .prop_readonly("isBought", &Land::isBought)
        .prop_readonly("isLeased", &Land::isLeased)
        .prop_readonly("isLeaseActive", &Land::isLeaseActive)
        .prop_readonly("isLeaseFrozen", &Land::isLeaseFrozen)
        .prop_readonly("isLeaseExpired", &Land::isLeaseExpired)
        .prop_readonly("isConvertedLand", &Land::isConvertedLand)     // legacy api
        .prop_readonly("isOwnerDataIsXUID", &Land::isOwnerDataIsXUID) // legacy api
        .prop_readonly("hasParentLand", &Land::hasParentLand)
        .prop_readonly("hasSubLand", &Land::hasSubLand)
        .prop_readonly("isSubLand", &Land::isSubLand)
        .prop_readonly("isParentLand", &Land::isParentLand)
        .prop_readonly("isMixLand", &Land::isMixLand)
        .prop_readonly("isOrdinaryLand", &Land::isOrdinaryLand)
        .prop_readonly("canCreateSubLand", &Land::canCreateSubLand)

        .method("getAABB", &Land::getAABB, jspp::binding::ReturnValuePolicy::kReferenceInternalPersistent)
        .method("getTeleportPos", &Land::getTeleportPos)
        .method("setTeleportPos", &Land::setTeleportPos)
        // .method("getPermTable", &Land::getPermTable)
        // .method("setPermTable", &Land::setPermTable)
        .method("getOwner", &Land::getOwner)
        .method("setOwner", &Land::setOwner)
        .method("getOwnershipKind", &Land::getOwnershipKind)
        .method("getRawOwner", &Land::getRawOwner) // legacy api
        .method("getPreviousOwner", &Land::getPreviousOwner)
        .method("getMembers", &Land::getMembers)
        .method("addLandMember", &Land::addLandMember)
        .method("removeLandMember", &Land::removeLandMember)
        .method("clearMembers", &Land::clearMembers)
        .method("getName", &Land::getName)
        .method("setName", &Land::setName)
        .method("getOriginalBuyPrice", &Land::getOriginalBuyPrice)
        .method("setOriginalBuyPrice", &Land::setOriginalBuyPrice)
        .method("getHoldType", &Land::getHoldType)
        .method("getLeaseState", &Land::getLeaseState)
        .method("getLeaseStartAt", &Land::getLeaseStartAt)
        .method("getLeaseEndAt", &Land::getLeaseEndAt)
        .method("isOwner", &Land::isOwner)
        .method("isMember", &Land::isMember)
        .method("isCollision", static_cast<bool (Land::*)(BlockPos const&, int) const>(&Land::isCollision))
        .method("getType", &Land::getType)
        .method("getParentLandID", &Land::getParentLandID)
        .method("getSubLandIDs", &Land::getSubLandIDs)
        .method("getNestedLevel", &Land::getNestedLevel)
        .method("getEffectiveRole", &Land::getEffectiveRole)
        .method("equals", &Land::operator==)
        .build();


} // namespace land::scripting