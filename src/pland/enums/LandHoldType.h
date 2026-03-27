#pragma once
#include <cstdint>


namespace land {

/**
 * 领地持有类型
 */
enum class LandHoldType : uint8_t {
    Bought = 0, // 购买模式
    Leased = 1, // 租赁模式
};


} // namespace land