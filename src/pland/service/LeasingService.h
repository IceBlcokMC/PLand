#pragma once
#include "ll/api/Expected.h"
#include "pland/Global.h"
#include "pland/aabb/LandAABB.h"

#include <memory>

class Player;

namespace land {
class Land;
enum class LandRecycleReason : uint8_t;
class LandRegistry;
class OrdinaryLandCreateSelector;

namespace service {
class SelectionService;
class LandPriceService;


class LeasingService {
    struct Impl;
    std::unique_ptr<Impl> impl;

    ll::Expected<> ensureLandLeased(std::shared_ptr<Land> const& land) const;

public:
    LeasingService(LandRegistry& registry, LandPriceService& priceService, SelectionService& selectionService);
    ~LeasingService();

    LD_DISABLE_COPY_AND_MOVE(LeasingService);

    /**
     * @brief 检查功能是否启用
     * @return 返回功能是否启用的状态
     */
    LDNDAPI bool enabled() const;

    /**
     * @brief 刷新领地的调度状态
     */
    LDAPI void refreshSchedule(std::shared_ptr<Land> const& land);

    /**
     * @brief 设置领地的开始时间
     * @param land 要设置的领地对象
     * @param date 要设置的开始时间
     */
    LDNDAPI ll::Expected<> setStartAt(std::shared_ptr<Land> const& land, std::chrono::system_clock::time_point date);

    /**
     * @brief 设置领地的结束时间
     * @param land 要设置的领地对象
     * @param date 要设置的结束时间
     */
    LDNDAPI ll::Expected<> setEndAt(std::shared_ptr<Land> const& land, std::chrono::system_clock::time_point date);

    /**
     * 强制冻结指定领地
     * @param land 要冻结的领地对象
     */
    LDNDAPI ll::Expected<> forceFreeze(std::shared_ptr<Land> const& land);

    /**
     * 强制回收指定领地
     * @param land 要回收的领地对象
     */
    LDNDAPI ll::Expected<> forceRecycle(std::shared_ptr<Land> const& land);

    /**
     * @brief 为指定领地添加时间
     * @param land 要添加时间的领地对象
     * @param seconds 要添加的时间（秒数）
     */
    LDNDAPI ll::Expected<> addTime(std::shared_ptr<Land> const& land, int seconds);

    /**
     * @brief 清理过期领地
     * @param daysOverdue 过期天数阈值
     * @return 返回清理的领地数量
     */
    LDNDAPI ll::Expected<int> cleanExpiredLands(int daysOverdue);

    /**
     * @brief 将租赁领地永久转为买断制领地
     * @param land 要转换状态的领地对象
     */
    LDNDAPI ll::Expected<> toBought(std::shared_ptr<Land> const& land);

    /**
     * @brief 将买断制领地转为租赁领地
     * @param land 要转换状态的领地对象
     * @param days 租赁天数
     */
    LDNDAPI ll::Expected<> toLeased(std::shared_ptr<Land> const& land, int days);

    /**
     * @brief 租赁领地
     * @param player 玩家对象引用
     * @param selector 普通领地创建选择器指针
     * @param days 租赁天数
     */
    LDNDAPI ll::Expected<std::shared_ptr<Land>>
            leaseLand(Player& player, OrdinaryLandCreateSelector* selector, int days);

    /**
     * @brief 续租领地
     * @param player 玩家对象引用
     * @param land 领地对象的共享指针
     * @param days 续租天数
     */
    LDNDAPI ll::Expected<> renewLease(Player& player, std::shared_ptr<Land> const& land, int days);

    /**
     * @brief 回收领地
     * @param land 领地对象的共享指针
     * @param reason 领地回收原因
     */
    ll::Expected<> recycleLand(std::shared_ptr<Land> const& land, LandRecycleReason reason);
};

} // namespace service
} // namespace land
