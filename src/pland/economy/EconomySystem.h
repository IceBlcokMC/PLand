#pragma once
#include "pland/Global.h"

#include <memory>

#include "econbridge/IEconomy.h"

class Player;
namespace mce {
class UUID;
}

namespace land {

class EconomySystem final {
    struct Impl;
    std::unique_ptr<Impl> impl;

    explicit EconomySystem();

public:
    LD_DISABLE_COPY_AND_MOVE(EconomySystem);

    LDNDAPI static EconomySystem& getInstance();

    LDAPI void initialize(); // 初始化经济系统
    LDAPI void reload();     // 重载经济系统（当 kit 改变时）

    LDNDAPI std::string getCostMessage(Player& player, llong amount) const;

    LDNDAPI std::shared_ptr<econbridge::IEconomy> get() const;

    inline std::shared_ptr<econbridge::IEconomy> operator->() const { return get(); }
};


} // namespace land
