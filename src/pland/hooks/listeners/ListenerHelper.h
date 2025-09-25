
#pragma once

#include "mc/platform/UUID.h"
#include "mc/world/level/block/BlockProperty.h"

#include "pland/PLand.h"
#include "pland/land/Land.h"
#include "pland/land/LandRegistry.h"

#define CANCEL_AND_RETURN_IF(COND, ...)                                                                                \
    if (COND) {                                                                                                        \
        __VA_ARGS__;                                                                                                   \
        ev.cancel();                                                                                                   \
        return;                                                                                                        \
    }

// clang-format off
#ifdef DISABLE_EVENT_TRACE
    #define EVENT_TRACE_PASS 0
    #define EVENT_TRACE_SKIP 0
    #define EVENT_TRACE_CANCEL 0
    #define EVENT_TRACE_LOG 0
    #define EVENT_TRACE(NAME, STATUS, MESSAGE, ...) (void(0))
#else
    #define STR_HELPER(x) #x
    #define STR(x)        STR_HELPER(x)

    inline constexpr std::string_view relative_file(std::string_view path) {
        // 截取 src/... 部分
        auto pos = path.find("src\\");
        if (pos != std::string_view::npos) return path.substr(pos);
        pos = path.find("src/");
        if (pos != std::string_view::npos) return path.substr(pos);

        // fallback: 只返回文件名
        pos = path.find_last_of("\\/");
        if (pos == std::string_view::npos) return path;
        return path.substr(pos + 1);
    }
    #define EVENT_TRACE_PASS "PASS" // 事件被放行(检查了权限)
    #define EVENT_TRACE_SKIP "SKIP" // 事件被跳过(未检查权限)
    #define EVENT_TRACE_CANCEL "CANCEL" // 事件被取消
    #define EVENT_TRACE_LOG "LOG" // 事件信息

    #define EVENT_TRACE(NAME, STATUS, MESSAGE, ...)                                                                             \
        logger->trace("[{}:" STR(__LINE__) "|" NAME "|" STATUS "] " MESSAGE, relative_file(__FILE__), __VA_ARGS__)
#endif
// clang-format on

namespace land {

// 共享的权限检查辅助函数
inline bool PreCheckLandExistsAndPermission(SharedLand const& ptr, mce::UUID const& uuid = mce::UUID::EMPTY()) {
    if (
        !ptr ||                                                       // 无领地
        (PLand::getInstance().getLandRegistry()->isOperator(uuid)) || // 管理员
        (ptr->getPermType(uuid) != LandPermType::Guest)               // 主人/成员
    ) {
        return true;
    }
    return false;
}

// 修复 BlockProperty 的 operator&
inline BlockProperty operator&(BlockProperty a, BlockProperty b) {
    return static_cast<BlockProperty>(static_cast<uint64_t>(a) & static_cast<uint64_t>(b));
}

} // namespace land
