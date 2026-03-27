#pragma once
#include <cstdint>


namespace land {

/**
 * 领地租赁回收模式
 */
enum class LeaseRecycleMode : uint8_t {
    TransferToSystem = 0, // 转移给系统
    Delete           = 1, // 删除
};


} // namespace land