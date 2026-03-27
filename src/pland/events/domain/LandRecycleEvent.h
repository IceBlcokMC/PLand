#pragma once
#include "pland/Global.h"
#include "pland/events/LandEventMixin.h"

#include "pland/enums/LandRecycleReason.h"

namespace land::event {


class LandRecycleEvent final : public LandEventMixin<ll::event::Event> {
    LandRecycleReason mReason;

public:
    explicit LandRecycleEvent(std::shared_ptr<Land> land, LandRecycleReason reason)
    : LandEventMixin(std::move(land)),
      mReason(reason) {}

    LDNDAPI LandRecycleReason reason() const;
};

} // namespace land::event
