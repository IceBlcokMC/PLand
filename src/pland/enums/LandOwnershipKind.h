#pragma once
#include <cstdint>


namespace land {

/**
 * 领地归属类型
 */
enum class LandOwnershipKind : uint8_t {
    Player           = 0, // 领地主为玩家 UUID
    PendingMigration = 1, // 领地主为 XUID 字符串，主人上线后自动迁移为 Player
    System           = 2, // 系统所有，租赁欠费回收后归系统账号
    Ownerless        = 3, // 无主领地，由领地管理员设置，领地主为空 UUID
};


} // namespace land