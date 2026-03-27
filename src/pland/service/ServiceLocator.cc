#include "ServiceLocator.h"

#include "LandHierarchyService.h"
#include "LandManagementService.h"
#include "LandPriceService.h"
#include "LeasingService.h"
#include "pland/PLand.h"
#include "pland/service/SelectionService.h"

#include <memory>

namespace land {
namespace service {

struct ServiceLocator::Impl {
    std::unique_ptr<LandHierarchyService>  mLandHierarchyService{nullptr};
    std::unique_ptr<LandPriceService>      mLandPriceService{nullptr};
    std::unique_ptr<SelectionService>      mSelectionService{nullptr};
    std::unique_ptr<LandManagementService> mLandManagementService{nullptr};
    std::unique_ptr<LeasingService>        mLeasingService{nullptr};

    void init(PLand& entry) {
        mLandHierarchyService  = std::make_unique<LandHierarchyService>(entry.getLandRegistry());
        mLandPriceService      = std::make_unique<LandPriceService>(*mLandHierarchyService);
        mSelectionService      = std::make_unique<SelectionService>(*entry.getSelectorManager());
        mLandManagementService = std::make_unique<LandManagementService>(
            entry.getLandRegistry(),
            *mSelectionService,
            *mLandHierarchyService,
            *mLandPriceService
        );
        mLeasingService =
            std::make_unique<LeasingService>(entry.getLandRegistry(), *mLandPriceService, *mSelectionService);
    }
    void destroy() {
        mLeasingService.reset();
        mLandManagementService.reset();
        mSelectionService.reset();
        mLandPriceService.reset();
        mLandHierarchyService.reset();
    }
};

ServiceLocator::ServiceLocator(PLand& entry) : impl(std::make_unique<Impl>()) { impl->init(entry); }
ServiceLocator::~ServiceLocator() { impl->destroy(); }

LandManagementService& ServiceLocator::getLandManagementService() const { return *impl->mLandManagementService; }
LandHierarchyService&  ServiceLocator::getLandHierarchyService() const { return *impl->mLandHierarchyService; }
LandPriceService&      ServiceLocator::getLandPriceService() const { return *impl->mLandPriceService; }
SelectionService&      ServiceLocator::getSelectionService() const { return *impl->mSelectionService; }
LeasingService&        ServiceLocator::getLeasingService() const { return *impl->mLeasingService; }

} // namespace service
} // namespace land
