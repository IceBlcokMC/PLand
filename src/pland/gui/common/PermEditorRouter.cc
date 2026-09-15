#include "PermEditorRouter.h"

#include "pland/PLand.h"
#include "pland/gui/BetterPermissionEditorGUI.h"
#include "pland/gui/LegacyPermTableEditor.h"
#include "pland/gui/utils/BackUtils.h"
#include "pland/land/repo/LandRegistry.h"
#include "pland/land/repo/PlayerSettings.h"

#include "mc/world/actor/player/Player.h"

#include <memory>
#include <utility>

namespace land::gui {

using SimpleForm = ll::form::SimpleForm;


void PermEditorRouter::open(
    Player&                    player,
    LandPermTable const&       initial,
    ApplyFn                    apply,
    SimpleForm::ButtonCallback back
) {
    auto& settings = PLand::getInstance().getLandRegistry().getOrCreatePlayerSettings(player.getUuid());
    auto  pref     = settings.permEditorPref.value_or(PermEditorPref::AskEach); // nullopt = 未设置

    switch (pref) {
    case PermEditorPref::LegacyJson: //
        openLegacy(player, initial, apply, std::move(back));
        break;
    case PermEditorPref::DDUI: //
        openDDUI(player, initial, apply);
        break;
    case PermEditorPref::AskEach: //
        showChooser(player, initial, std::move(apply), std::move(back));
        break;
    }
}

void PermEditorRouter::openDDUI(Player& player, LandPermTable const& initial, ApplyFn const& apply) {
    using Editor = BetterPermissionEditorGUI;

    auto table = std::make_shared<LandPermTable>(initial);
    Editor::sendTo(
        player,
        *table,
        [table, apply](Editor::EnvPermField const& field, bool value) {
            field.set(table->environment, value);
            apply(*table);
        },
        [table,
         apply](Editor::RolePermField const& permField, Editor::RolePermEntryField const& entryField, bool value) {
            auto entry = permField.get(table->role);
            entryField.set(entry, value);
            permField.set(table->role, entry);
            apply(*table);
        }
    );
}

void PermEditorRouter::openLegacy(
    Player&                    player,
    LandPermTable const&       initial,
    ApplyFn const&             apply,
    SimpleForm::ButtonCallback back
) {
    LegacyPermTableEditor::sendTo(
        player,
        initial,
        [apply](Player&, LandPermTable newTable) { apply(newTable); },
        std::move(back)
    );
}

void PermEditorRouter::showChooser(
    Player&                    player,
    LandPermTable              initial,
    ApplyFn                    apply,
    SimpleForm::ButtonCallback back
) {
    auto localeCode = player.getLocaleCode();

    auto fm = SimpleForm{};
    fm.setTitle("[PLand] | 选择权限编辑器"_trl(localeCode));
    fm.setContent("请选择要使用的权限编辑表单"_trl(localeCode));
    fm.appendButton(
        "新版 DDUI 编辑器"_trl(localeCode),
        "textures/ui/icon_recipe_nature",
        "path",
        [initial, apply](Player& self) { openDDUI(self, initial, apply); }
    );
    fm.appendButton(
        "传统 JSON UI 编辑器"_trl(localeCode),
        "textures/ui/book_write_default",
        "path",
        [initial, apply, back](Player& self) { openLegacy(self, initial, apply, back); }
    );
    if (back) {
        back_utils::injectBackButton(fm, back);
    }
    fm.sendTo(player);
}

} // namespace land::gui
