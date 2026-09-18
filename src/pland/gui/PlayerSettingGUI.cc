#include "PlayerSettingGUI.h"

#include "pland/PLand.h"
#include "pland/land/repo/LandRegistry.h"
#include "pland/land/repo/PlayerSettings.h"
#include "pland/utils/FeedbackUtils.h"

#include "ll/api/form/CustomForm.h"

#include "mc/world/actor/player/Player.h"

#include <algorithm>
#include <string>
#include <vector>

namespace land::gui {
using namespace ll::form;

void PlayerSettingGUI::sendTo(Player& player) {
    auto& setting = PLand::getInstance().getLandRegistry().getOrCreatePlayerSettings(player.getUuid());

    auto localeCode = player.getLocaleCode();

    std::vector<std::string> prefOptions{
        "传统 JSON UI"_trl(localeCode),
        "新版 DDUI"_trl(localeCode),
        "每次询问"_trl(localeCode),
    };

    CustomForm fm(("[PLand] | 玩家设置"_trl(localeCode)));

    fm.appendToggle("showEnterLandTitle", "是否显示进入领地提示"_trl(localeCode), setting.showEnterLandTitle);
    fm.appendToggle("showBottomContinuedTip", "是否持续显示底部提示"_trl(localeCode), setting.showBottomContinuedTip);
    fm.appendDropdown(
        "permEditorPref",
        "权限编辑表单偏好"_trl(localeCode),
        prefOptions,
        static_cast<size_t>(setting.permEditorPref.value_or(PermEditorPref::AskEach))
    );

    fm.sendTo(player, [prefOptions](Player& pl, CustomFormResult res, FormCancelReason) {
        if (!res) {
            return;
        }

        auto& registry = PLand::getInstance().getLandRegistry();
        auto& setting  = registry.getOrCreatePlayerSettings(pl.getUuid());

        setting.showEnterLandTitle     = std::get<uint64_t>(res->at("showEnterLandTitle"));
        setting.showBottomContinuedTip = std::get<uint64_t>(res->at("showBottomContinuedTip"));

        auto prefPref = PermEditorPref::AskEach;
        if (auto slot = res->find("permEditorPref"); slot != res->end()) {
            if (auto* text = std::get_if<std::string>(&slot->second)) {
                if (auto it = std::ranges::find(prefOptions, *text); it != prefOptions.end()) {
                    prefPref = static_cast<PermEditorPref>(std::distance(prefOptions.begin(), it));
                }
            }
        }
        setting.permEditorPref = prefPref;

        registry.savePlayerSettings();

        feedback_utils::sendText(pl, "设置已保存"_trl(pl.getLocaleCode()));
    });
}


} // namespace land::gui
