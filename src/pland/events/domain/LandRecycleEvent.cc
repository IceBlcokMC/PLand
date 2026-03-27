#include "LandRecycleEvent.h"
#include "pland/events/Helper.h"

namespace land::event {

LandRecycleReason LandRecycleEvent::reason() const { return mReason; }

IMPLEMENT_EVENT_EMITTER(LandRecycleEvent)

} // namespace land::event
