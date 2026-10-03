#pragma once
#include "AdvisorBase.h"

#include <memory>

namespace land ::advisor {
enum class AdvisorResult : uint8_t;

class AdvisorRegistry {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    AdvisorRegistry();
    ~AdvisorRegistry();

    // hasAdvisor
    // addAdvisor
    // removeAdvisor
    // emplaceAdvisor

    /// for EventInterceptor
    [[nodiscard]] AdvisorResult dispatch(ll::event::Event& event, std::shared_ptr<Land> const& land);
};

} // namespace land::advisor
