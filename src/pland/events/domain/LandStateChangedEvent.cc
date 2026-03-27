#include "LandStateChangedEvent.h"
#include "pland/events/Helper.h"

namespace land::event {

LeaseState LandStateChangedEvent::oldState() const { return mOldState; }
LeaseState LandStateChangedEvent::newState() const { return mNewState; }

IMPLEMENT_EVENT_EMITTER(LandStateChangedEvent)

} // namespace land::event
