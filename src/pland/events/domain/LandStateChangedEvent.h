#pragma once
#include "pland/Global.h"
#include "pland/enums/LeaseState.h"
#include "pland/events/LandEventMixin.h"

namespace land::event {

class LandStateChangedEvent final : public LandEventMixin<ll::event::Event> {
    LeaseState mOldState;
    LeaseState mNewState;

public:
    explicit LandStateChangedEvent(std::shared_ptr<Land> land, LeaseState oldState, LeaseState newState)
    : LandEventMixin(std::move(land)),
      mOldState(oldState),
      mNewState(newState) {}

    LDNDAPI LeaseState oldState() const;

    LDNDAPI LeaseState newState() const;
};

} // namespace land::event
