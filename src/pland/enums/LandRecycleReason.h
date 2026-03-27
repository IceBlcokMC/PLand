#pragma once
#include <cstdint>


namespace land {


enum class LandRecycleReason : uint8_t {
    LeaseExpired = 0, // 租赁到期
    Inactivity   = 1, // 闲置
    ForceRecycle = 2, // 强制回收
};


}