#include "HookRegistry.h"
#include "pland/interceptor/internal/EventInterceptor.h"
#include "pland/interceptor/internal/InterceptorConfig.h"
#include "pland/interceptor/internal/helper/InterceptorHelper.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/actor/item/ExperienceOrb.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/projectile/AbstractArrow.h"
#include "mc/world/actor/projectile/Arrow.h"
#include "mc/world/actor/projectile/ThrownTrident.h"
#include "mc/world/level/BlockSource.h"

namespace land::internal::interceptor {

// https://github.com/IceBlcokMC/PLand/issues/193

namespace {

template <typename T>
[[nodiscard]] inline bool canPickupInLand(T* self, Player& player) {
    auto& registry = PLand::getInstance().getLandRegistry();
    auto  land     = registry.getLandAt(self->getPosition(), self->getDimensionId());
    return hasRolePermission<&RolePerms::allowPlayerPickupItem>(land, player.getUuid());
}

} // namespace

#define DEAL_PLAYER_TOUCH_HOOK(H_NAME, H_CLASS)                                                                        \
    LL_TYPE_INSTANCE_HOOK(                                                                                             \
        H_NAME,                                                                                                        \
        ll::memory::HookPriority::Normal,                                                                              \
        H_CLASS,                                                                                                       \
        &H_CLASS::$playerTouch,                                                                                        \
        void,                                                                                                          \
        ::Player& player                                                                                               \
    ) {                                                                                                                \
        if (!canPickupInLand(this, player)) {                                                                          \
            return;                                                                                                    \
        }                                                                                                              \
        origin(player);                                                                                                \
    }

DEAL_PLAYER_TOUCH_HOOK(ExperienceOrbPlayerTouchHook, ExperienceOrb)
DEAL_PLAYER_TOUCH_HOOK(ThrownTridentPlayerTouchHook, ThrownTrident)
DEAL_PLAYER_TOUCH_HOOK(ArrowPlayerTouchHook, Arrow)
DEAL_PLAYER_TOUCH_HOOK(AbstractArrowPlayerTouchHook, AbstractArrow)

#undef DEAL_PLAYER_TOUCH_HOOK

LAND_REGISTER_HOOK([](EventInterceptor& interceptor) {
    interceptor.registerHookIf<&InterceptorConfig::Hooks::ExperienceOrbPlayerTouchHook, ExperienceOrbPlayerTouchHook>();
    interceptor.registerHookIf<&InterceptorConfig::Hooks::ThrownTridentPlayerTouchHook, ThrownTridentPlayerTouchHook>();
    interceptor.registerHookIf<&InterceptorConfig::Hooks::ArrowPlayerTouchHook, ArrowPlayerTouchHook>();
    interceptor.registerHookIf<&InterceptorConfig::Hooks::AbstractArrowPlayerTouchHook, AbstractArrowPlayerTouchHook>();
});

} // namespace land::internal::interceptor