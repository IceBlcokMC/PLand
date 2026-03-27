#include "LandEventPublisher.h"

#include "pland/events/domain/LandStateChangedEvent.h"
#include "pland/events/domain/MemberChangedEvent.h"
#include "pland/events/domain/OwnerChangedEvent.h"

#include <ll/api/event/EventBus.h>


namespace land::observer {

void LandEventPublisher::onOwnerChanged(
    std::shared_ptr<Land> const& land,
    mce::UUID const&             oldOwner,
    mce::UUID const&             newOwner
) {
    if (oldOwner != newOwner) {
        ll::event::EventBus::getInstance().publish(event::OwnerChangedEvent{land, oldOwner, newOwner});
    }
}

void LandEventPublisher::onMemberAdded(std::shared_ptr<Land> const& land, mce::UUID const& member) {
    ll::event::EventBus::getInstance().publish(event::MemberChangedEvent{land, member, true});
}

void LandEventPublisher::onMemberRemoved(std::shared_ptr<Land> const& land, mce::UUID const& member) {
    ll::event::EventBus::getInstance().publish(event::MemberChangedEvent{land, member, false});
}

void LandEventPublisher::onMembersCleared(std::shared_ptr<Land> const& land) {
    ll::event::EventBus::getInstance().publish(event::MembersClearedEvent{land});
}

void LandEventPublisher::onLeaseStateChanged(
    std::shared_ptr<Land> const& land,
    LeaseState                   oldState,
    LeaseState                   newState
) {
    if (oldState != newState) {
        ll::event::EventBus::getInstance().publish(event::LandStateChangedEvent{land, oldState, newState});
    }
}


} // namespace land::observer