#pragma once
#include <utility>

#include "ll/api/event/Cancellable.h"
#include "ll/api/event/player/PlayerEvent.h"

#include "pland/Global.h"
#include "pland/events/LandEventMixin.h"


namespace land::event {


class IPlayerTransferLandEvent : public LandEventMixin<ll::event::PlayerEvent> {
    mce::UUID mNewOwner;

public:
    explicit IPlayerTransferLandEvent(Player& player, std::shared_ptr<Land> land, mce::UUID newOwner)
    : LandEventMixin(std::move(land), player),
      mNewOwner(newOwner) {}

    // 空 UUID 表示管理员将领地设为无主领地。
    LDNDAPI mce::UUID newOwner() const;
};

class PlayerTransferLandBeforeEvent final : public ll::event::Cancellable<IPlayerTransferLandEvent> {
    using ll::event::Cancellable<IPlayerTransferLandEvent>::Cancellable;
};
class PlayerTransferLandAfterEvent final : public IPlayerTransferLandEvent {
    using IPlayerTransferLandEvent::IPlayerTransferLandEvent;
};


} // namespace land::event
