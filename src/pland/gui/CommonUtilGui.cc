#include "pland/gui/CommonUtilGUI.h"
#include "ll/api/form/CustomForm.h"
#include "ll/api/form/FormBase.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/ecs/WeakEntityRef.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"
#include "pland/Global.h"
#include "pland/PLand.h"
#include "pland/land/Land.h"
#include "pland/land/LandRegistry.h"
#include <algorithm>
#include <string>


namespace land {
using namespace ll::form;


void ChooseOnlinePlayerUtilGUI::sendTo(
    Player&                          player,
    ChoosePlayerCall const&          callback,
    BackSimpleForm<>::ButtonCallback back
) {
    auto fm = BackSimpleForm<>{std::move(back)};
    fm.setTitle(PLUGIN_NAME + ("| 选择玩家"_trf(player)));

    ll::service::getLevel()->forEachPlayer([callback, &fm](Player& target) {
        if (target.isSimulatedPlayer()) {
            return true; // ignore
        }

        fm.appendButton(target.getRealName(), [callback, weakRef = target.getWeakEntity()](Player& self) {
            if (auto target = weakRef.tryUnwrap<Player>()) {
                callback(self, target);
            }
        });
        return true;
    });

    fm.sendTo(player);
}


void EditStringUtilGUI::sendTo(
    Player&            player,
    std::string const& title,        // 标题
    std::string const& text,         // 提示
    std::string const& defaultValue, // 默认值
    EditStringResult   callback      // 回调
) {
    CustomForm fm(PLUGIN_NAME + title);
    fm.appendInput("str", text, "string", defaultValue);
    fm.sendTo(player, [cb = std::move(callback)](Player& pl, CustomFormResult res, FormCancelReason) {
        if (!res.has_value()) {
            return;
        }
        cb(pl, std::get<std::string>(res->at("str")));
    });
}


} // namespace land