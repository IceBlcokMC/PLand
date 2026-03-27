#pragma once
#include "pland/Global.h"

#include <memory>


namespace land {
class PLand;
namespace service {

class LandHierarchyService;
class LandManagementService;
class LandPriceService;
class SelectionService;
class LeasingService;

class ServiceLocator {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    ServiceLocator(PLand& entry);
    ~ServiceLocator();

    LD_DISABLE_COPY_AND_MOVE(ServiceLocator);

    LDNDAPI LandManagementService& getLandManagementService() const;

    LDNDAPI LandHierarchyService& getLandHierarchyService() const;

    LDNDAPI LandPriceService& getLandPriceService() const;

    LDNDAPI SelectionService& getSelectionService() const;

    LDNDAPI LeasingService& getLeasingService() const;
};

} // namespace service
} // namespace land
