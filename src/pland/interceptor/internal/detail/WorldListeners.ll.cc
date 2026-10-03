#include "pland/interceptor/internal/EventInterceptor.h"
#include "pland/interceptor/internal/InterceptorConfig.h"
#include "pland/interceptor/internal/helper/EventTrace.h"
#include "pland/interceptor/internal/helper/InterceptorHelper.h"

#include "ll/api/event/world/FireSpreadEvent.h"
#include "mc/world/level/dimension/DimensionType.h"

#include <ll/api/event/EventBus.h>

namespace land::internal::interceptor {

void EventInterceptor::setupLLWorldListeners() {
    auto registry = &PLand::getInstance().getLandRegistry();
    auto bus      = &ll::event::EventBus::getInstance();

    registerListenerIf<&InterceptorConfig::Listeners::FireSpreadEvent>([bus, registry]() {
        return bus->emplaceListener<ll::event::FireSpreadEvent>([registry](ll::event::FireSpreadEvent& ev) {
            auto& pos = ev.pos();

            auto land = registry->getLandAt(pos, ev.blockSource().getDimensionId());
            if (!hasEnvironmentPermission<&EnvironmentPerms::allowFireSpread>(land)) {
                ev.cancel();
            }
        });
    });
}

} // namespace land::internal::interceptor
