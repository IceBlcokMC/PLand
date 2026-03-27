#include "MemberChangedEvent.h"
#include "pland/events/Helper.h"


namespace land::event {


mce::UUID MemberChangedEvent::target() const { return mTarget; }

bool MemberChangedEvent::isAdd() const { return mIsAdd; }

IMPLEMENT_EVENT_EMITTER(MemberChangedEvent)

IMPLEMENT_EVENT_EMITTER(MembersClearedEvent)

} // namespace land::event