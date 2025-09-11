#include "DebugShapeHandle.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/world/phys/AABB.h"
#include "pland/Global.h"
#include "pland/aabb/LandAABB.h"
#include "pland/infra/draw/IDrawHandle.h"
#include "pland/land/Land.h"
#include <cassert>
#include <cstdint>
#include <mc/deps/core/utility/AutomaticID.h>
#include <memory>
#include <unordered_map>
#include <utility>

#include "fake_header/DebugShape.h"

#include "windows.h"

namespace land {

#ifndef _MSC_VER
#error "This code only supports MSVC"
#endif

inline constexpr auto DebugShapeModuleName = L"DebugShape.dll";
inline constexpr auto DebugShapeBoundsBoxCtorSymbol =
    "??0BoundsBox@extension@debug_shape@@QEAA@AEBVAABB@@AEBVColor@mce@@@Z";

// MSVC placement new abi
using DebugShapeBoundsBoxCtor =
    void (*)(debug_shape::extension::BoundsBox* thiz, AABB const& aabb, mce::Color const& color);

bool isDebugShapeLoaded() { return GetModuleHandle(DebugShapeModuleName) != nullptr; }

struct BoundsBoxDeleter {
    void operator()(debug_shape::extension::BoundsBox* ptr) {
        ptr->~BoundsBox();
        ::operator delete(ptr);
    }
};

using UniqueBoundsBox = std::unique_ptr<debug_shape::extension::BoundsBox, BoundsBoxDeleter>;

UniqueBoundsBox newBoundsBox(AABB const& aabb, mce::Color const& color = mce::Color::WHITE()) {
    if (!isDebugShapeLoaded()) {
        throw std::runtime_error("DebugShape.dll not loaded");
    }
    auto raw = GetProcAddress(GetModuleHandle(DebugShapeModuleName), DebugShapeBoundsBoxCtorSymbol);
    if (!raw) {
        throw std::runtime_error("Failed to get address of DebugShapeBoundsBoxCtor");
    }

    auto ctor = reinterpret_cast<DebugShapeBoundsBoxCtor>(raw);

    void* memory = ::operator new(sizeof(debug_shape::extension::BoundsBox));
    ctor(reinterpret_cast<debug_shape::extension::BoundsBox*>(memory), aabb, color);

    return UniqueBoundsBox(reinterpret_cast<debug_shape::extension::BoundsBox*>(memory));
}

inline AABB toMinecraftAABB(LandPos const& min, LandPos const& max) {
    return AABB{
        Vec3{min.x + 0.08, min.y + 0.08, min.z + 0.08},
        Vec3{max.x + 0.98, max.y + 0.98, max.z + 0.98}
    };
}
inline AABB toMinecraftAABB(LandAABB const& aabb) { return toMinecraftAABB(aabb.min, aabb.max); }

inline GeoId allocatedID() {
    static uint64_t next{1};
    return GeoId{next++};
}


struct DebugShapeHandle::Impl {
    std::unordered_map<GeoId, UniqueBoundsBox>  mShapes;     // 绘制的形状
    std::unordered_map<LandID, UniqueBoundsBox> mLandShapes; // 绘制的领地
};


// interface
DebugShapeHandle::DebugShapeHandle() : impl_(std::make_unique<Impl>()) {}
DebugShapeHandle::~DebugShapeHandle() = default;

GeoId DebugShapeHandle::draw(LandAABB const& aabb, DimensionType dimId, mce::Color const& color) {
    auto box = newBoundsBox(toMinecraftAABB(aabb), color);
    box->setColor(color);
    box->draw(dimId);

    auto id = allocatedID();
    impl_->mShapes.emplace(id, std::move(box));
    return id;
}

void DebugShapeHandle::draw(std::shared_ptr<Land> const& land, mce::Color const& color) {
    if (impl_->mLandShapes.contains(land->getId())) {
        return; // 已经绘制过
    }
    auto box = newBoundsBox(toMinecraftAABB(land->getAABB()), color);
    box->setColor(color);
    box->draw(land->getDimensionId());
    impl_->mLandShapes.emplace(land->getId(), std::move(box));
}

void DebugShapeHandle::remove(GeoId id) {
    auto iter = impl_->mShapes.find(id);
    if (iter != impl_->mShapes.end()) {
        impl_->mShapes.erase(iter);
    }
}

void DebugShapeHandle::remove(LandID landId) {
    auto iter = impl_->mLandShapes.find(landId);
    if (iter != impl_->mLandShapes.end()) {
        impl_->mLandShapes.erase(iter);
    }
}

void DebugShapeHandle::remove(std::shared_ptr<Land> land) { remove(land->getId()); }

void DebugShapeHandle::clear() {
    impl_->mShapes.clear();
    impl_->mLandShapes.clear();
}

void DebugShapeHandle::clearLand() { impl_->mLandShapes.clear(); }
bool DebugShapeHandle::isDebugShapeLoaded() { return land::isDebugShapeLoaded(); }


} // namespace land