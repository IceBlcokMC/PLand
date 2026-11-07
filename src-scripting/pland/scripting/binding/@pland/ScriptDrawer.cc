#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "pland/Global.h"
#include "pland/PLand.h"
#include "pland/drawer/DrawHandleManager.h"
#include "pland/drawer/IDrawerHandle.h"
#include "pland/land/Land.h"

// GeoId (aka uint64_t) -> double
template <>
struct jspp::binding::TypeConverter<land::drawer::GeoId> {
    static Local<Value> toJs(land::drawer::GeoId id, ReturnValuePolicy /* policy */, Local<Value> const& /* parent */) {
        return Number::newNumber(static_cast<double>(id));
    }
    static land::drawer::GeoId toCpp(Local<Value> const& value) {
        using type = land::scripting::VectorMemberType_t<land::drawer::GeoId, &land::drawer::GeoId::value>;
        return land::drawer::GeoId{value.asNumber().getValueAs<type>()};
    }
};

namespace land::scripting {

namespace ScriptDrawerImpl {

inline DrawHandleManager* get() { return PLand::getInstance().getDrawHandleManager(); }

auto drawAABB(Player& player, LandAABB const& aabb, DimensionType dim, mce::Color const& color) {
    if (auto h = get()->tryGetHandle(player.getUuid())) {
        return h->draw(aabb, dim, color);
    }
    return land::drawer::GeoId{};
}

void drawLand(Player& player, std::shared_ptr<Land> const& land, mce::Color const& color) {
    if (auto h = get()->tryGetHandle(player.getUuid())) {
        h->draw(land, color);
    }
}

void removeByGeoId(Player& player, land::drawer::GeoId id) {
    if (auto h = get()->tryGetHandle(player.getUuid())) {
        h->remove(id);
    }
}

void removeByLandId(Player& player, LandID id) {
    if (auto h = get()->tryGetHandle(player.getUuid())) {
        h->remove(id);
    }
}

void removeByLand(Player& player, std::shared_ptr<Land> const& land) {
    if (auto h = get()->tryGetHandle(player.getUuid())) {
        h->remove(land);
    }
}

void clearAll(Player& player) {
    if (auto h = get()->tryGetHandle(player.getUuid())) {
        h->clear();
    }
}

void clearLandOnly(Player& player) {
    if (auto h = get()->tryGetHandle(player.getUuid())) {
        h->clearLand();
    }
}

} // namespace ScriptDrawerImpl

decltype(Modules::kScriptDrawer) Modules::kScriptDrawer = //
    jspp::binding::defClass<void>("shapeDrawer")
        .func("drawAABB", &ScriptDrawerImpl::drawAABB)
        .func("drawLand", &ScriptDrawerImpl::drawLand)
        .func("removeByGeoId", &ScriptDrawerImpl::removeByGeoId)
        .func("removeByLandId", &ScriptDrawerImpl::removeByLandId)
        .func("removeByLand", &ScriptDrawerImpl::removeByLand)
        .func("clearAll", &ScriptDrawerImpl::clearAll)
        .func("clearLandOnly", &ScriptDrawerImpl::clearLandOnly)
        .build();

} // namespace land::scripting