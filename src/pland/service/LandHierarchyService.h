#pragma once
#include "ll/api/Expected.h"
#include "pland/Global.h"

#include <ll/api/coro/Generator.h>
#include <memory>

namespace land {
class LandRegistry;
class Land;
} // namespace land

namespace land {
namespace service {

/**
 * @brief 子领地层级管理
 * @see https://github.com/engsr6982/PLand/issues/18
 *
 * @note 子领地分为 3 种状态:
 * @note 父领地：(根领地) 作为整个领地树的根节点（没有父领地、有子领地）
 * @note 混合领地：同时包含子领地和普通领地（同时拥有父领地和子领地）
 * @note 子领地：领地树的末端（即：拥有父领地、没有子领地）
 */
class LandHierarchyService {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    LandHierarchyService(LandRegistry& registry);
    ~LandHierarchyService();
    LD_DISABLE_COPY_AND_MOVE(LandHierarchyService);

    LDNDAPI ll::Expected<> ensureBoughtLand(std::shared_ptr<Land> const& land);

    /**
     * 创建子领地关系
     * @note sub 必须为新创建的普通领地，被 parent 完整包含
     */
    LDNDAPI ll::Expected<> attachSubLand(std::shared_ptr<Land> const& parent, std::shared_ptr<Land> const& sub);

    /**
     * 仅仅移除一个子领地（将其从父领地剥离并删除）
     * @note 仅接受 isSubLand() == true 的领地
     * @note 被删除的目标领地必须为领地树的末端，不支持混合领地、根领地
     */
    LDNDAPI ll::Expected<> deleteSubLand(std::shared_ptr<Land> const& sub);

    /**
     * 删除当前领地,并递归删除此领地下的所有子领地
     * @note 仅接受 isParentLand() || isMixLand() == true 的领地
     */
    LDNDAPI ll::Expected<> deleteLandRecursive(std::shared_ptr<Land> const& land);

    /**
     * 删除父领地，并将所有子领地晋升为普通领地
     * @note 仅接受 isParentLand() == true 的领地
     */
    LDNDAPI ll::Expected<> deleteParentLandAndPromoteChildren(std::shared_ptr<Land> const& parent);

    /**
     * 删除一个混合领地（有父有子），并将其子领地移交给爷爷领地
     * @note 仅接受 isMixLand() == true 的领地
     */
    LDNDAPI ll::Expected<> deleteMixLandAndTransferChildren(std::shared_ptr<Land> const& mixLand);


    /**
     * @brief 获取父领地
     */
    LDNDAPI std::shared_ptr<Land> getParent(std::shared_ptr<Land> const& land) const;

    /**
     * @brief 获取子领地
     */
    LDNDAPI std::vector<std::shared_ptr<Land>> getSubLands(std::shared_ptr<Land> const& land) const;

    /**
     * @brief 获取当前领地到最顶层领地的层级深度
     * @return 根领地返回 0，其子领地依次递增
     * @note 动态沿 parent 链计算，无缓存
     */
    LDNDAPI int getNestedLevel(std::shared_ptr<Land> const& land) const;

    /**
     * 更新领地层级
     * @param root 根领地(从此领地往下修改)
     * @param rootLevel 根领地层级
     */
    LDAPI void updateLevels(std::shared_ptr<Land> const& root, int rootLevel);

    /**
     * @brief 获取根领地(即最顶层的普通领地 isOrdinaryLand() == true)
     */
    LDNDAPI std::shared_ptr<Land> getRoot(std::shared_ptr<Land> const& land) const;

    /**
     * @brief 获取从当前领地的根领地出发的所有子领地（包含根和当前领地）
     */
    LDNDAPI std::unordered_set<std::shared_ptr<Land>> getFamilyTree(std::shared_ptr<Land> const& land) const;

    /**
     * @note 遍历领地的所有下级子领地(包含自身)
     */
    LDNDAPI ll::coro::Generator<std::shared_ptr<Land>> getDescendants(std::shared_ptr<Land> const& land) const;

    /**
     * @note 遍历领地的所有上级父领地(包含自身)
     */
    LDNDAPI ll::coro::Generator<std::shared_ptr<Land>> getAncestors(std::shared_ptr<Land> const& land) const;

    /**
     * @brief 获取当前领地及其所有上级父领地（包含自身）
     */
    LDNDAPI std::unordered_set<std::shared_ptr<Land>> getSelfAndAncestors(std::shared_ptr<Land> const& land) const;

    /**
     * @brief 获取当前领地及其所有下级子领地（包含自身）
     */
    LDNDAPI std::unordered_set<std::shared_ptr<Land>> getSelfAndDescendants(std::shared_ptr<Land> const& land) const;

    LDNDAPI ll::Expected<> ensureSubLandLegal(std::shared_ptr<Land> const& parent, std::shared_ptr<Land> const& sub);
};

} // namespace service
} // namespace land
