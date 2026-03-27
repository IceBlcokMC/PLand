#pragma once
#include <cstdint>

namespace land {

/**
 * 领地租赁状态
 */
enum class LeaseState : uint8_t {
    None    = 0, // 非租赁领地 / 无状态
    Active  = 1, // 正常期
    Frozen  = 2, // 冻结期
    Expired = 3, // 已到期(已回收)
};

} // namespace land