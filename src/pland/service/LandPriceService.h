#pragma once
#include "pland/Global.h"


#include <ll/api/Expected.h>
#include <memory>

namespace land {
class LandAABB;
}
namespace land {
class Land;
}
namespace land {
namespace service {
class LandHierarchyService;

class LandPriceService final {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    LD_DISABLE_COPY_AND_MOVE(LandPriceService)
    explicit LandPriceService(LandHierarchyService& service);
    ~LandPriceService();

    /**
     * 递归计算领地价值
     * @param land 需要计算的领地
     * @param func 自定义处理函数(若为空，则默认对所有领地进行累加)
     * @return 计算结果
     */
    LDNDAPI int64_t calculatePriceRecursively(
        std::shared_ptr<Land>                                                         land,
        std::function<bool(std::shared_ptr<Land> const& land, int64_t& price)> const& func = nullptr
    ) const;

    struct PriceResult {
        int64_t               mOriginalPrice{0};          // 原始价格
        int64_t               mDiscountedPrice{0};        // 折扣后价格
        std::optional<double> mMultipliers{std::nullopt}; // 倍率

        PriceResult() = default;
        explicit PriceResult(
            int64_t               originalPrice,
            int64_t               discountedPrice,
            std::optional<double> multipliers = std::nullopt
        )
        : mOriginalPrice(originalPrice),
          mDiscountedPrice(discountedPrice),
          mMultipliers(multipliers) {}
    };

    /**
     * 获取普通领地报价
     * @param range 领地范围
     * @param dimId 领地所在维度
     * @param is3D 是否为三维领地
     * @note 此函数可能返回底层表达式解析失败异常，请勿发送给玩家
     */
    LDNDAPI ll::Expected<PriceResult> getOrdinaryLandPrice(LandAABB const& range, int dimId, bool is3D) const;

    /**
     * 获取子领地报价
     * @param range 领地范围
     * @param dimId 领地所在维度
     * @note 此函数可能返回底层表达式解析失败异常，请勿发送给玩家
     */
    LDNDAPI ll::Expected<PriceResult> getSubLandPrice(LandAABB const& range, int dimId) const;


    /**
     * 获取单个领地退款金额
     * @param land 领地
     */
    LDNDAPI int64_t getRefundAmount(std::shared_ptr<Land> const& land) const;

    /**
     * 递归计算领地退款金额
     * @param land 领地
     */
    LDNDAPI int64_t getRefundAmountRecursively(std::shared_ptr<Land> const& land) const;


    // ================================
    // 租赁接口
    // ================================

    struct LeaseCostDetail {
        long long dailyRent{0};   // 日租金
        long long overdueDays{0}; // 逾期天数
        long long baseAmount{0};  // 基础金额
        long long penalty{0};     // 违约金
        long long total{0};       // 总金额
    };
    /**
     * @brief 计算每日租金
     * @param range 领地范围(AABB包围盒)
     * @param dimId 维度ID
     * @param is3D 是否为3D空间
     * @return 预期的每日租金值，可能包含错误信息
     */
    LDNDAPI ll::Expected<long long> calculateDailyRent(LandAABB const& range, int dimId, bool is3D) const;

    /**
     * @brief 计算租赁总成本
     * @param range 领地范围(AABB包围盒)
     * @param dimId 维度ID
     * @param is3D 是否为3D空间
     * @param days 租赁天数
     * @return 预期的租赁总成本，可能包含错误信息
     */
    LDNDAPI ll::Expected<long long> calculateLeaseCost(LandAABB const& range, int dimId, bool is3D, int days) const;

    /**
     * @brief 计算续租成本
     * @param land 领地对象的共享指针
     * @param days 续租天数
     * @return 续租成本详情
     */
    LDNDAPI LeaseCostDetail calculateRenewCost(std::shared_ptr<Land> const& land, int days) const;

private:
    static ll::Expected<> ensureEconomySystemAvailable();

    ll::Expected<PriceResult> executeFormula(LandAABB const& range, int dimId, std::string const& formula) const;
};

} // namespace service
} // namespace land
