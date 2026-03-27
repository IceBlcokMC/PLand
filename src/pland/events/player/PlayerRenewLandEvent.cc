#include "PlayerRenewLandEvent.h"
#include "pland/events/Helper.h"

namespace land::event {

long long PlayerRenewLandEvent::payMoney() const { return mPayMoney; }
int       PlayerRenewLandEvent::days() const { return mDays; }

IMPLEMENT_EVENT_EMITTER(PlayerRenewLandEvent)

} // namespace land::event
