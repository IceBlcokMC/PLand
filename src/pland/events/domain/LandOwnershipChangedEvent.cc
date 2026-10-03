#include "LandOwnershipChangedEvent.h"
#include "pland/events/Helper.h"


namespace land::event {


LandOwnershipKind LandOwnershipChangedEvent::oldKind() const { return mOldKind; }

LandOwnershipKind LandOwnershipChangedEvent::newKind() const { return mNewKind; }

IMPLEMENT_EVENT_EMITTER(LandOwnershipChangedEvent)

} // namespace land::event