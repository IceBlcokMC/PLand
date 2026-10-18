#include "AdvisorRegistry.h"

#include "AdvisorBase.h"
#include "AdvisorImpl.h"

namespace land::interceptor::advisor {

struct AdvisorRegistry::Impl {};

AdvisorRegistry::AdvisorRegistry() : impl(std::make_unique<Impl>()) {}
AdvisorRegistry::~AdvisorRegistry() = default;

bool AdvisorRegistry::hasAdvisor(AdvisorId id) const {
    return false; // TODO: impl
}

bool AdvisorRegistry::addAdvisor(std::shared_ptr<AdvisorBase> const& advisor) {
    // advisor->mId = 0 // TODO: alloc id
    return false;
}

bool AdvisorRegistry::removeAdvisor(AdvisorId id) {
    return false; // TODO: impl
}

AdvisorResult AdvisorRegistry::dispatch(ll::event::Event const& event, std::shared_ptr<Land> const& land) {
    return AdvisorResult::Continue; // TODO: impl
}


} // namespace land::interceptor::advisor