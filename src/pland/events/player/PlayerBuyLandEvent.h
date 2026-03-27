#pragma once
#include "pland/Global.h"
#include "pland/enums/LandType.h"
#include "pland/events/LandEventMixin.h"

#include "ll/api/event/player/PlayerEvent.h"
#include <ll/api/event/Cancellable.h>


namespace land {
class Land;
namespace event {

class PlayerBuyLandBeforeEvent final : public ll::event::Cancellable<ll::event::PlayerEvent> {
    int64_t& mPayMoney;
    LandType mLandType;

public:
    explicit PlayerBuyLandBeforeEvent(Player& player, int64_t& payMoney, LandType landType)
    : Cancellable(player),
      mPayMoney(payMoney),
      mLandType(landType) {}

    LDNDAPI int64_t& payMoney() const;

    LDNDAPI LandType landType() const;
};

class PlayerBuyLandAfterEvent final : public LandEventMixin<ll::event::PlayerEvent> {
    int64_t mPayMoney;

public:
    explicit PlayerBuyLandAfterEvent(Player& player, const std::shared_ptr<Land>& land, int64_t payMoney)
    : LandEventMixin(land, player),
      mPayMoney(payMoney) {}

    LDNDAPI int64_t payMoney() const;
};


} // namespace event
} // namespace land
