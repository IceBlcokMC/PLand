#pragma once
#include "AdvisorBase.h"
#include "AdvisorImpl.h"

#include <memory>

namespace land::interceptor::advisor {
enum class AdvisorResult : uint8_t;

class AdvisorRegistry {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    AdvisorRegistry();
    ~AdvisorRegistry();

    bool hasAdvisor(AdvisorId id) const;

    bool addAdvisor(std::shared_ptr<AdvisorBase> const& advisor);

    bool removeAdvisor(AdvisorId id);

    bool removeAdvisor(std::shared_ptr<AdvisorBase> const& advisor) {
        if (advisor) {
            return removeAdvisor(advisor->getId());
        }
        return false;
    }

    template <
        std::derived_from<ll::event::Event> T,
        AdvisorFor<T>                       H,
        std::derived_from<AdvisorBase>      Impl = AdvisorImpl<T, H>,
        typename... Args>
    [[nodiscard]] auto emplaceAdvisor(H&& handler, Args&&... args) {
        auto ptr = Impl::make(std::forward<H>(handler), std::forward<Args>(args)...);
        if (!addAdvisor(ptr)) {
            ptr = nullptr;
        }
        return ptr;
    }

    /// for EventInterceptor
    [[nodiscard]] AdvisorResult dispatch(ll::event::Event const& event, std::shared_ptr<Land> const& land);
};

} // namespace land::interceptor::advisor
