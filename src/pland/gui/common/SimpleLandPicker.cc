#include "SimpleLandPicker.h"

#include "pland/gui/utils/BackUtils.h"
#include "pland/land/Land.h"
#include "pland/utils/TimeUtils.h"

namespace land {
namespace gui {

void SimpleLandPicker::sendTo(
    Player&                              player,
    std::vector<std::shared_ptr<Land>>   data,
    Callback                             callback,
    ll::form::SimpleForm::ButtonCallback backTo
) {
    auto f = ll::form::SimpleForm{};

    auto localeCode = player.getLocaleCode();
    f.setTitle("[PLand] | 领地选择器"_trl(localeCode));

    if (backTo) {
        back_utils::injectBackButton(f, std::move(backTo));
    }

    for (auto& land : data) {
        std::string leaseContent = "";
        if (land->isLeased()) {
            auto state = land->getLeaseState();
            if (state == LeaseState::Active) {
                leaseContent = time_utils::formatRemaining(land->getLeaseEndAt());
            } else if (state == LeaseState::Frozen) {
                leaseContent = " | §e已冻结§r"_trl(localeCode);
            } else {
                leaseContent = " | §c租赁过期§r"_trl(localeCode);
            }
        }
        f.appendButton(
            "[{}] {}§r\n维度: {}{}§r"_trl(
                localeCode,
                land->getId(),
                land->getName(),
                land->getDimensionId(),
                leaseContent
            ),
            "textures/ui/icon_recipe_nature",
            "path",
            [weak = std::weak_ptr{land}, callback](Player& player) {
                if (auto land = weak.lock()) {
                    callback(player, land);
                }
            }
        );
    }

    f.sendTo(player);
}

} // namespace gui
} // namespace land