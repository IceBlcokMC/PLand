#pragma once
#include "pland/PLand.h"
#include "pland/land/Config.h"
#include "pland/land/Land.h"
#include "pland/land/repo/LandRegistry.h"
#include "pland/reflect/TypeName.h"

#include "EventTrace.h"

#include <memory>

namespace land::internal::interceptor {

/**
 * 检查玩家是否拥有特权
 * @param land 领地
 * @param uuid 玩家UUID
 * @return 是否拥有特权
 */
inline bool hasPrivilege(std::shared_ptr<Land> const& land, mce::UUID const& uuid) {
    TRACE_ADD_SCOPE("hasPrivilege");

    TRACE_LOG("land={}", land ? land->getName() : "nullptr");
    if (!land) {
        TRACE_LOG("land not exist, bypass");
        return true; // 领地不存在 => 放行
    }

    if (land->isLeaseFrozen()) {
        // 如果冻结, 则仅放行管理员
        TRACE_LOG("land is frozen, check for operator");
        goto admin;
    }

    if (land->isOwner(uuid)) {
        // 短路: 如果是主人则直接放行, 避免无意义的查管理表
        TRACE_LOG("owner allowed");
        return true;
    }

admin:
    bool isOperator = PLand::getInstance().getLandRegistry().isOperator(uuid);
    TRACE_LOG("check for operator table, result: {}", isOperator ? "allowed" : "denied");
    return isOperator;
}

/**
 * 检查环境权限
 * @tparam Member 权限字段
 * @param land 领地
 * @return 是否有权限
 */
template <bool EnvironmentPerms::* Member>
inline bool hasEnvironmentPermission(std::shared_ptr<Land> const& land) {
    TRACE_ADD_SCOPE(reflect::extractFunctionSignature(__FUNCSIG__));

    TRACE_LOG("land={}", land ? land->getName() : "nullptr");
    if (!land) {
        TRACE_LOG("land not exist, bypass");
        return true; // 领地不存在 => 放行
    }

    bool result = land->getPermTable().environment.*Member;
    TRACE_LOG("{}={}", reflect::extractTemplateInnerLeafName(__FUNCSIG__), result ? "allowed" : "denied");
    return result;
}

inline bool _hasMemberOrGuestPermission(
    std::shared_ptr<Land> const& land,
    mce::UUID const&             uuid,
    RolePerms::Entry RolePerms::* pointer
) {
    assert(pointer);
    TRACE_ADD_SCOPE("_hasMemberOrGuestPermission");

    TRACE_LOG("land={}", land ? land->getName() : "nullptr");
    if (!land) {
        TRACE_LOG("land not exist, bypass");
        return true; // 领地不存在 => 放行
    }

    auto entry = land->getPermTable().role.*pointer;
    TRACE_LOG("unknown: member={}, actor={}", entry.member ? "allowed" : "denied", entry.actor ? "allowed" : "denied");

    if (land->isLeaseFrozen()) {
        TRACE_LOG("land is frozen, fallback to actor");
        return entry.actor; // 如果冻结, 不再允许 Member 特权，退化为 Actor
    }

    if (entry.actor) {
        TRACE_LOG("actor allowed");
        return true; // 短路: 如果访客允许，那么不必再查成员
    }
    if (!entry.member) {
        TRACE_LOG("member denied");
        return false; // 短路: 如果成员直接不允许，那么没有查表的必要
    }
    bool isMember = land->isMember(uuid);
    TRACE_LOG("check for member table, result: {}", isMember ? "allowed" : "denied");
    return isMember;
}
/**
 * 检查玩家成员访客权限
 */
template <RolePerms::Entry RolePerms::* Member>
inline bool hasMemberOrGuestPermission(std::shared_ptr<Land> const& land, mce::UUID const& uuid) {
    TRACE_ADD_SCOPE(reflect::extractFunctionSignature(__FUNCSIG__));
    TRACE_LOG("check permission for: {}", reflect::extractTemplateInnerLeafName(__FUNCSIG__));
    return _hasMemberOrGuestPermission(land, uuid, Member);
}


/**
 * 检查玩家角色权限
 * @tparam Member 权限字段
 * @param land 领地
 * @param uuid 玩家 UUID
 * @return 是否有权限
 */
template <RolePerms::Entry RolePerms::* Member>
inline bool hasRolePermission(std::shared_ptr<Land> const& land, mce::UUID const& uuid) {
    TRACE_ADD_SCOPE(reflect::extractFunctionSignature(__FUNCSIG__));
    if (hasPrivilege(land, uuid)) return true;             // 领地不存在 / 管理员 / 主人 => 放行
    return hasMemberOrGuestPermission<Member>(land, uuid); // 成员 / 访客
}

/**
 * 检查访客是否有权限
 * @tparam Member 权限字段
 * @param land 领地
 * @note 此函数仅用于解决某些无法归类的问题
 * @note 如：铜傀儡 + 容器权限, 且容器权限归为角色权限
 * @note 现有权限模型下铜傀儡不是有效的角色类型, 权限也无法划分为环境类权限
 */
template <RolePerms::Entry RolePerms::* Member>
inline bool hasGuestPermission(std::shared_ptr<Land> const& land) {
    TRACE_ADD_SCOPE(reflect::extractFunctionSignature(__FUNCSIG__));

    TRACE_LOG("land={}", land ? land->getName() : "nullptr");
    if (!land) {
        TRACE_LOG("land not exist, bypass");
        return true; // 领地不存在 => 放行
    }

    auto entry = land->getPermTable().role.*Member;
    TRACE_LOG(
        "check actor permission('{}'), result: {}",
        reflect::extractTemplateInnerLeafName(__FUNCSIG__),
        entry.actor ? "allowed" : "denied"
    );
    return entry.actor;
}


} // namespace land::internal::interceptor
