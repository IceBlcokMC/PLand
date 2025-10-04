#pragma once
#include "pland/Global.h"

#include "ll/api/event/Event.h"


namespace land {
struct Config;
}

namespace land::events::inline infra {


class ConfigReloadEvent final : public ll::event::Event {
    Config& mConfig;

public:
    LDAPI explicit ConfigReloadEvent(Config& config);

    LDNDAPI Config& getConfig();
};


} // namespace land::events::inline infra