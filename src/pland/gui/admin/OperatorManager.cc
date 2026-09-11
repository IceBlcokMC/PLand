#include "OperatorManager.h"

#include "LandOwnerPicker.h"
#include "pland/PLand.h"
#include "pland/gui/LandManagerGUI.h"
#include "pland/gui/common/PermEditorRouter.h"
#include "pland/gui/common/AdvancedLandPicker.h"
#include "pland/land/Land.h"
#include "pland/land/LandTemplatePermTable.h"
#include "pland/land/repo/LandContext.h"
#include "pland/land/repo/LandRegistry.h"
#include "pland/utils/FeedbackUtils.h"

#include "mc/deps/ecs/WeakEntityRef.h"
#include "mc/server/ServerPlayer.h"

#include "ll/api/service/PlayerInfo.h"
#include "pland/gui/common/SimpleInputForm.h"
#include "pland/gui/utils/BackUtils.h"

#include <ll/api/form/SimpleForm.h>

namespace land::gui {


void OperatorManager::sendMainMenu(Player& player) {
    auto localeCode = player.getLocaleCode();

    if (!PLand::getInstance().getLandRegistry().isOperator(player.getUuid())) {
        feedback_utils::sendErrorText(player, "无权限访问此表单"_trl(localeCode));
        return;
    }

    auto fm = ll::form::SimpleForm{};

    fm.setTitle("[PLand] | 领地管理"_trl(localeCode));
    fm.setContent("请选择您要进行的操作"_trl(localeCode));

    fm.appendButton("管理脚下领地"_trl(localeCode), "textures/ui/free_download", "path", [](Player& self) {
        auto lands = PLand::getInstance().getLandRegistry().getLandAt(self.getPosition(), self.getDimensionId());
        if (!lands) {
            feedback_utils::sendErrorText(self, "您当前所处位置没有领地"_trl(self.getLocaleCode()));
            return;
        }
        LandManagerGUI::sendMainMenu(self, lands);
    });
    fm.appendButton("管理玩家领地"_trl(localeCode), "textures/ui/FriendsIcon", "path", [](Player& self) {
        LandOwnerPicker::sendTo(self, static_cast<void (*)(Player&, mce::UUID)>(&sendAdvancedLandPicker), sendMainMenu);
    });
    fm.appendButton("管理指定领地"_trl(localeCode), "textures/ui/magnifyingGlass", "path", [](Player& self) {
        sendLandSelectModeMenu(self);
    });
    fm.appendButton("编辑默认权限"_trl(localeCode), "textures/ui/icon_map", "path", [](Player& self) {
        auto saved = std::make_shared<bool>(false);
        auto ref   = self.getEntityContext().getWeakRef();

        PermEditorRouter::open(
            self,
            PLand::getInstance().getLandRegistry().getLandTemplatePermTable().get(),
            [saved, ref](LandPermTable const& table) {
                PLand::getInstance().getLandRegistry().getLandTemplatePermTable().set(table);
                if (*saved) {
                    return;
                }
                *saved = true;
                if (auto* sp = ref.tryUnwrap<ServerPlayer>().as_ptr()) {
                    feedback_utils::sendText(*sp, "权限表已更新"_trl(sp->getLocaleCode()));
                }
            },
            sendMainMenu
        );
    });

    fm.sendTo(player);
}
void OperatorManager::sendLandSelectModeMenu(Player& player) {
    auto localeCode = player.getLocaleCode();

    ll::form::SimpleForm fm;
    fm.setTitle("[PLand] | 管理指定领地 | 选择方式"_trl(localeCode));
    fm.appendButton(
        "浏览全部领地"_trl(localeCode),
        "textures/ui/achievements_pause_menu_icon",
        "path",
        [](Player& self) { sendAdvancedLandPicker(self, PLand::getInstance().getLandRegistry().getLands()); }
    );
    fm.appendButton("按领地 ID 查找"_trl(localeCode), "textures/ui/magnifyingGlass", "path", [](Player& self) {
        sendLandIdSearchForm(self);
    });
    back_utils::injectBackButton<sendMainMenu>(fm);
    fm.sendTo(player);
}
void OperatorManager::sendLandIdSearchForm(Player& player) {
    auto localeCode = player.getLocaleCode();
    SimpleInputForm::sendTo(
        player,
        "[PLand] | 管理指定领地 | 按领地 ID 查找"_trl(localeCode),
        "请输入领地 ID"_trl(localeCode),
        "",
        [](Player& self, std::string input) {
            auto localeCode = self.getLocaleCode();
            try {
                LandID id = std::stoll(input);
                if (auto land = PLand::getInstance().getLandRegistry().getLand(id)) {
                    LandManagerGUI::sendMainMenu(self, land);
                } else {
                    feedback_utils::sendErrorText(self, "未找到领地 ID 为 {} 的领地"_trl(localeCode, input));
                }
            } catch (...) {
                feedback_utils::sendErrorText(self, "解析失败，非法的领地ID"_trl(localeCode));
            }
        }
    );
}

void OperatorManager::sendAdvancedLandPicker(Player& player, mce::UUID targetPlayer) {
    sendAdvancedLandPicker(player, PLand::getInstance().getLandRegistry().getLands(targetPlayer));
}

void OperatorManager::sendAdvancedLandPicker(Player& player, std::vector<std::shared_ptr<Land>> lands) {
    AdvancedLandPicker::sendTo(
        player,
        lands,
        [](Player& self, std::shared_ptr<Land> ptr) { LandManagerGUI::sendMainMenu(self, ptr); },
        back_utils::wrapCallback<sendMainMenu>()
    );
}


} // namespace land::gui