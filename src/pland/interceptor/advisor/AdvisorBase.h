#pragma once
#include "pland/Global.h"

#include <memory>

namespace ll::event {
class Event;
}

namespace land {
class Land;
}

namespace land::interceptor::advisor {

enum class AdvisorResult : uint8_t { Allow, Deny, Continue };

enum class AdvisorPriority : uint16_t {
    Highest = 0,
    High    = 100,
    Normal  = 200,
    Low     = 300,
    Lowest  = 400,
};

using AdvisorId = uint64_t;

inline constexpr AdvisorId kInvalidAdvisorId = 0;

class AdvisorBase {
    AdvisorId       mId{kInvalidAdvisorId};
    AdvisorPriority mPriority{AdvisorPriority::Normal};

    friend class AdvisorRegistry;

protected:
    LDAPI explicit AdvisorBase(AdvisorPriority priority = AdvisorPriority::Normal);

public:
    virtual ~AdvisorBase() = default;

    AdvisorBase(AdvisorBase&&)                 = delete;
    AdvisorBase(AdvisorBase const&)            = delete;
    AdvisorBase& operator=(AdvisorBase&&)      = delete;
    AdvisorBase& operator=(AdvisorBase const&) = delete;

    [[nodiscard]] constexpr AdvisorId getId() const noexcept { return mId; }

    [[nodiscard]] constexpr AdvisorPriority getPriority() const noexcept { return mPriority; }

    [[nodiscard]] constexpr bool operator==(AdvisorBase const& other) const noexcept { return mId == other.mId; }

    [[nodiscard]] constexpr std::strong_ordering operator<=>(AdvisorBase const& other) const noexcept {
        if (mPriority != other.mPriority) {
            return mPriority <=> other.mPriority;
        }
        return mId <=> other.mId;
    }

    [[nodiscard]] virtual AdvisorResult handle(ll::event::Event const& event, std::shared_ptr<Land> const& land) = 0;
};

using AdvisorPtr = std::shared_ptr<AdvisorBase>;

} // namespace land::interceptor::advisor