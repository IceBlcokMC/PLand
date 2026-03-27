#pragma once
#include "pland/Global.h"
#include "pland/events/LandEventMixin.h"

#include "ll/api/event/player/PlayerEvent.h"

namespace land::event {

class PlayerLeaseLandEvent final : public LandEventMixin<ll::event::PlayerEvent> {
    long long mPayMoney;
    int       mDays;

public:
    explicit PlayerLeaseLandEvent(Player& player, std::shared_ptr<Land> const& land, long long payMoney, int days)
    : LandEventMixin(land, player),
      mPayMoney(payMoney),
      mDays(days) {}

    LDNDAPI long long payMoney() const;

    LDNDAPI int days() const;
};

} // namespace land::event
