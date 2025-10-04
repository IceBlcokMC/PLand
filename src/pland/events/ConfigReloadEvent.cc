#include "ConfigReloadEvent.h"
#include "pland/events/Helper.h"

namespace land::events::inline infra {


ConfigReloadEvent::ConfigReloadEvent(land::Config& config) : mConfig(config) {}

Config& ConfigReloadEvent::getConfig() { return mConfig; }


IMPLEMENT_EVENT_EMITTER(ConfigReloadEvent)

} // namespace land::events::inline infra