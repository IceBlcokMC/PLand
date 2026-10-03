#include "AdvisorRegistry.h"

#include "AdvisorBase.h"
#include "AdvisorImpl.h"

namespace land ::advisor {

struct AdvisorRegistry::Impl {};

AdvisorRegistry::AdvisorRegistry() : impl(std::make_unique<Impl>()) {}
AdvisorRegistry::~AdvisorRegistry() = default;


AdvisorResult AdvisorRegistry::dispatch(ll::event::Event& event, std::shared_ptr<Land> const& land) {
    return AdvisorResult::Continue; // TODO: impl
}


} // namespace land::advisor