#include "PlayerLeaseLandEvent.h"
#include "pland/events/Helper.h"

namespace land::event {

long long PlayerLeaseLandEvent::payMoney() const { return mPayMoney; }
int       PlayerLeaseLandEvent::days() const { return mDays; }

IMPLEMENT_EVENT_EMITTER(PlayerLeaseLandEvent)

} // namespace land::event
