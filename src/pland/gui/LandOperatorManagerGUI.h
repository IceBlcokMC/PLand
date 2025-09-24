#pragma once
#include "pland/Global.h"
#include "pland/land/Land.h"

class Player;

namespace land {


/**
 * @brief 领地操作员GUI
 */
class LandOperatorManagerGUI {
public:
    LandOperatorManagerGUI() = delete;

    LDAPI static void sendMainMenu(Player& player);

    using ChoosePlayerCallback = std::function<void(Player& self, mce::UUID const& target)>;
    LDAPI static void sendChoosePlayerFromDb(Player& player, ChoosePlayerCallback callback);

    LDAPI static void sendChooseLandGUI(Player& player, mce::UUID const& targetPlayer);
    LDAPI static void sendChooseLandAdvancedGUI(Player& player, std::vector<SharedLand> lands);
};


} // namespace land