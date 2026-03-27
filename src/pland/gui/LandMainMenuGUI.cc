#include "LandMainMenuGUI.h"
#include "LandManagerGUI.h"
#include "NewLandGUI.h"
#include "PlayerSettingGUI.h"
#include "common/SimpleLandPicker.h"
#include "pland/PLand.h"
#include "pland/gui/LandTeleportGUI.h"
#include "pland/land/Config.h"
#include "pland/land/repo/LandRegistry.h"
#include "utils/BackUtils.h"

#include <ll/api/form/SimpleForm.h>


namespace land::gui {


void LandMainMenuGUI::sendTo(Player& player) {
    auto localeCode = player.getLocaleCode();

    auto fm = ll::form::SimpleForm{};
    fm.setTitle("[PLand] | 领地菜单"_trl(localeCode));
    fm.setContent("欢迎使用 Pland 领地管理插件\n\n请选择一个功能"_trl(localeCode));

    fm.appendButton("新建领地"_trl(localeCode), "textures/ui/anvil_icon", "path", [](Player& pl) {
        NewLandGUI::sendChooseLandDim(pl);
    });

    fm.appendButton("管理领地"_trl(localeCode), "textures/ui/icon_spring", "path", [](Player& pl) {
        SimpleLandPicker::sendTo(
            pl,
            PLand::getInstance().getLandRegistry().getLands(pl.getUuid()),
            LandManagerGUI::sendMainMenu,
            gui::back_utils::wrapCallback<sendTo>()
        );
    });

    if (ConfigProvider::isLandTeleportEnabled()
        || PLand::getInstance().getLandRegistry().isOperator(player.getUuid())) {
        fm.appendButton("领地传送"_trl(localeCode), "textures/ui/icon_recipe_nature", "path", [](Player& pl) {
            LandTeleportGUI::sendTo(pl);
        });
    }

    fm.appendButton("个人设置"_trl(localeCode), "textures/ui/icon_setting", "path", [](Player& pl) {
        PlayerSettingGUI::sendTo(pl);
    });

    fm.appendButton("关闭"_trl(localeCode), "textures/ui/cancel", "path");
    fm.sendTo(player);
}


} // namespace land::gui