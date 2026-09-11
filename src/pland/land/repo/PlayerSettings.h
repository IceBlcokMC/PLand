#pragma once
#include <cstdint>
#include <optional>

namespace land {

// 权限编辑表单偏好
enum class PermEditorPref : std::uint8_t {
    LegacyJson = 0, // 传统 JSON UI
    DDUI       = 1, // 新版 DDUI 表单
    AskEach    = 2, // 每次询问
};

struct PlayerSettings {
    bool                          showEnterLandTitle{true};     // 是否显示进入领地提示
    bool                          showBottomContinuedTip{true}; // 是否持续显示底部提示
    std::optional<PermEditorPref> permEditorPref{};             // 权限编辑表单偏好; nullopt = 未设置, 按 AskEach 处理
};

} // namespace land