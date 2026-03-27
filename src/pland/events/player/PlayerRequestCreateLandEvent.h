#pragma once
#include "pland/Global.h"
#include "pland/enums/LandType.h"

#include <ll/api/event/player/PlayerEvent.h>

namespace land::event {


class PlayerRequestCreateLandEvent final : public ll::event::PlayerEvent {
public:
    explicit PlayerRequestCreateLandEvent(Player& player, LandType type) : PlayerEvent(player), mType(type) {}

    LDNDAPI LandType type() const;

private:
    LandType mType;
};


} // namespace land::event