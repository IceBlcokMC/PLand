#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "pland/aabb/LandAABB.h"

namespace land::scripting {

decltype(Modules::kScriptAABB) Modules::kScriptAABB = //
    jspp::binding::defClass<LandAABB>("LandAABB")
        .ctor([](LandPos min, LandPos max) { return jspp::binding::factory::newNativeInstanceFor<LandAABB>(min, max); })
        .prop("min", &LandAABB::min)
        .prop("max", &LandAABB::max)

        .prop_readonly("spanX", &LandAABB::getSpanX)
        .prop_readonly("spanY", &LandAABB::getSpanY)
        .prop_readonly("spanZ", &LandAABB::getSpanZ)
        .prop_readonly("blockCountX", &land::LandAABB::getBlockCountX)
        .prop_readonly("blockCountY", &land::LandAABB::getBlockCountY)
        .prop_readonly("blockCountZ", &land::LandAABB::getBlockCountZ)
        .prop_readonly("square", &land::LandAABB::getSquare)
        .prop_readonly("volume", &land::LandAABB::getVolume)

        .method("canonicalize", &LandAABB::canonicalize)
        .method("toString", &land::LandAABB::toString)
        .method("hasPos", &LandAABB::hasPos)
        .method("expanded", &LandAABB::expanded, jspp::binding::ReturnValuePolicy::kMove)
        .method("isOnInnerBoundary", &LandAABB::isOnInnerBoundary)
        .method("isOnOuterBoundary", &LandAABB::isOnOuterBoundary)
        .method("isAboveLand", &LandAABB::isAboveLand)
        .method("equals", &LandAABB::operator==)

        .func("isCollision", &LandAABB::isCollision)
        .func("getDistanceSq", &LandAABB::getDistanceSq)
        .func("isComplisWithMinSpacing", &LandAABB::isComplisWithMinSpacing)
        .func("isContain", &LandAABB::isContain)
        .func("getMinSpacing", &LandAABB::getMinSpacing)
        .build();

} // namespace land::scripting