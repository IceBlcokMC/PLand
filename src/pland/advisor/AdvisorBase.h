#pragma once
#include "pland/Global.h"


#include <memory>

namespace ll::event {
class Event;
}

namespace land {
class Land;
}

namespace land::advisor {

enum class AdvisorResult : uint8_t { Allow, Deny, Continue };

enum class AdvisorPriority : uint16_t {
    Highest = 0,
    High    = 100,
    Normal  = 200,
    Low     = 300,
    Lowest  = 400,
};

using AdvisorId = uint64_t;

class AdvisorBase {
    AdvisorId       mId{0};
    AdvisorPriority mPriority{AdvisorPriority::Normal};

    friend class AdvisorRegistry;

protected:
    LDAPI explicit AdvisorBase(AdvisorPriority priority = AdvisorPriority::Normal);

public:
    virtual ~AdvisorBase() = default;

    LDNDAPI AdvisorId getId() const noexcept;

    LDNDAPI AdvisorPriority getPriority() const noexcept;

    [[nodiscard]] virtual AdvisorResult handle(ll::event::Event& event, std::shared_ptr<Land> const& land) = 0;
};

} // namespace land::advisor