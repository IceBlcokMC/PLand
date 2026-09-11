#pragma once
#include "pland/Global.h"

#include "ll/api/form/SimpleForm.h"

#include <functional>

class Player;

namespace land {
struct LandPermTable;
}

namespace land::gui {

/**
 * @brief 权限编辑器路由
 *
 * 按玩家偏好 (PlayerSettings::permEditorPref) 打开权限编辑表单：
 * - LegacyJson -> LegacyPermTableEditor（提交时整表回调）
 * - DDUI       -> BetterPermissionEditorGUI（每字段即时回调）
 * - AskEach / 未设置 -> 独立路由 SimpleForm 每次询问
 */
class PermEditorRouter {
public:
    PermEditorRouter() = delete;

    /// 每次变更(DDUI)/提交(Legacy)后以最新整表调用，由调用方负责持久化
    using ApplyFn = std::function<void(LandPermTable const&)>;

    LDAPI static void open(
        Player&                              player,
        LandPermTable const&                 initial,
        ApplyFn                              apply,
        ll::form::SimpleForm::ButtonCallback back = {}
    );

private:
    static void openDDUI(Player& player, LandPermTable const& initial, ApplyFn const& apply);
    static void openLegacy(
        Player&                              player,
        LandPermTable const&                 initial,
        ApplyFn const&                       apply,
        ll::form::SimpleForm::ButtonCallback back
    );
    static void showChooser(
        Player&                              player,
        LandPermTable                        initial,
        ApplyFn                              apply,
        ll::form::SimpleForm::ButtonCallback back
    );
};

} // namespace land::gui
