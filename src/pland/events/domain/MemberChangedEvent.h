#pragma once
#include "pland/Global.h"
#include "pland/events/LandEventMixin.h"

#include "ll/api/event/Event.h"

#include "mc/platform/UUID.h"

#include <memory>
#include <utility>


namespace land ::event {


class MemberChangedEvent final : public LandEventMixin<ll::event::Event> {
    mce::UUID mTarget;
    bool      mIsAdd; // true if added, false if removed

public:
    explicit MemberChangedEvent(std::shared_ptr<Land> land, mce::UUID target, bool isAdd)
    : LandEventMixin(std::move(land)),
      mTarget(target),
      mIsAdd(isAdd) {}

    LDNDAPI mce::UUID target() const;

    LDNDAPI bool isAdd() const;
};

class MembersClearedEvent final : public LandEventMixin<ll::event::Event> {
public:
    explicit MembersClearedEvent(std::shared_ptr<Land> land) : LandEventMixin(std::move(land)) {}
};


} // namespace land::event