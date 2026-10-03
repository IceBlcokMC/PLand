#include "AdvisorBase.h"

namespace land::advisor {

AdvisorBase::AdvisorBase(AdvisorPriority priority) : mPriority(priority) {}

AdvisorId AdvisorBase::getId() const noexcept { return mId; }

AdvisorPriority AdvisorBase::getPriority() const noexcept { return mPriority; }

} // namespace land::advisor