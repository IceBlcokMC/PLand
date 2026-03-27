#pragma once
#include "ll/api/event/Event.h"

namespace land::events::inline infra {


class ConfigReloadEvent final : public ll::event::Event {
public:
    explicit ConfigReloadEvent() {}
};


} // namespace land::events::inline infra