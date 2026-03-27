#pragma once
#include "pland/Global.h"
#include "pland/selector/land/OrdinaryLandCreateSelector.h"

class Player;

namespace land {

class OrdinaryLandCreateSelector;
class LandResizeSelector;
class SubLandCreateSelector;

namespace gui {

class LandBuyGUI {
public:
    /**
     * @brief 购买领地(调用对应的impl函数)
     */
    LDAPI static void sendTo(Player& player);

    /**
     * 进行路由跳转
     */
    LDAPI static void _chooseHoldType(Player& player, OrdinaryLandCreateSelector* selector);

    /**
     * @brief 购买普通领地
     */
    static void _impl(Player& player, OrdinaryLandCreateSelector* selector);

    /**
     * @brief 选择租赁天数
     */
    static void _chooseLeaseDays(Player& player, OrdinaryLandCreateSelector* selector);

    static void _confirmLeaseDays(
        Player&                     player,
        OrdinaryLandCreateSelector* selector,
        int                         days,
        int64_t                     totalPrice,
        std::string                 baseContent
    );

    /**
     * @brief 购买新的领地范围
     */
    static void _impl(Player& player, LandResizeSelector* selector);

    /**
     * @brief 购买子领地
     */
    static void _impl(Player& player, SubLandCreateSelector* selector);
};

} // namespace gui
} // namespace land
