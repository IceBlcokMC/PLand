#include "OperatorManager.h"

#include "LandOwnerPicker.h"
#include "pland/PLand.h"
#include "pland/gui/LandManagerGUI.h"
#include "pland/gui/common/AdvancedLandPicker.h"
#include "pland/gui/common/PermEditorRouter.h"
#include "pland/land/Config.h"
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

#include <utility>

namespace land::gui {


void OperatorManager::sendMainMenu(Player& player) {
    auto localeCode = player.getLocaleCode();

    auto& registry = PLand::getInstance().getLandRegistry();
    if (!registry.isOperator(player.getUuid())) {
        feedback_utils::sendErrorText(player, "无权限访问此表单"_trl(localeCode));
        return;
    }

    auto fm = ll::form::SimpleForm{};

    fm.setTitle("[PLand] | 领地管理"_trl(localeCode));
    fm.setContent("请选择您要进行的操作"_trl(localeCode));

    // 脚下没有领地时不提供入口
    if (auto current = registry.getLandAt(player.getPosition(), player.getDimensionId())) {
        fm.appendButton("管理脚下领地"_trl(localeCode), "textures/ui/free_download", "path", [current](Player& self) {
            LandManagerGUI::sendMainMenu(self, current);
        });
    }

    fm.appendButton("管理玩家领地"_trl(localeCode), "textures/ui/FriendsIcon", "path", [](Player& self) {
        LandOwnerPicker::sendTo(self, static_cast<void (*)(Player&, mce::UUID)>(&sendAdvancedLandPicker), sendMainMenu);
    });

    // 空列表与未启用的功能不提供入口
    if (ConfigProvider::isOwnerlessEnabled()) {
        std::vector<std::shared_ptr<Land>> ownerlessLands;
        registry.forEachLand([&](std::shared_ptr<Land> const& land) {
            if (land->isOwnerless()) {
                ownerlessLands.push_back(land);
            }
            return true;
        });
        if (!ownerlessLands.empty()) {
            fm.appendButton(
                "管理无主领地"_trl(localeCode),
                "textures/ui/deop.png",
                "path",
                [ownerlessLands](Player& self) { sendAdvancedLandPicker(self, ownerlessLands); }
            );
        }
    }

    {
        std::vector<std::shared_ptr<Land>> pendingLands;
        registry.forEachLand([&](std::shared_ptr<Land> const& land) {
            if (land->getOwnershipKind() == LandOwnershipKind::PendingMigration) {
                pendingLands.push_back(land);
            }
            return true;
        });
        if (!pendingLands.empty()) {
            fm.appendButton(
                "管理待迁移领地"_trl(localeCode),
                "textures/ui/recipe_book_icon",
                "path",
                [pendingLands](Player& self) { sendAdvancedLandPicker(self, pendingLands); }
            );
        }
    }
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
        [](Player& self) {
            std::vector<std::shared_ptr<Land>> lands;
            PLand::getInstance().getLandRegistry().forEachLand([&](std::shared_ptr<Land> const& land) {
                lands.push_back(land);
                return true;
            });
            sendAdvancedLandPicker(self, lands);
        }
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
        std::move(lands),
        [](Player& self, std::shared_ptr<Land> ptr) { LandManagerGUI::sendMainMenu(self, std::move(ptr)); },
        back_utils::wrapCallback<sendMainMenu>()
    );
}


} // namespace land::gui
