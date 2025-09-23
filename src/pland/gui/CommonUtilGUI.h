#pragma once
#include "pland/Global.h"
#include "pland/gui/form/BackSimpleForm.h"


class Player;

namespace land {


class ChooseOnlinePlayerUtilGUI {
public:
    ChooseOnlinePlayerUtilGUI() = delete;

    using ChoosePlayerCall = std::function<void(Player&, Player& choosedPlayer)>;
    LDAPI static void
    sendTo(Player& player, ChoosePlayerCall const& callback, BackSimpleForm<>::ButtonCallback back = {});
};


class EditStringUtilGUI {
public:
    EditStringUtilGUI() = delete;

    using EditStringResult = std::function<void(Player& self, std::string result)>;
    LDAPI static void sendTo(
        Player&            player,
        std::string const& title,       // 标题
        std::string const& text,        // 提示
        std::string const& defaultValu, // 默认值
        EditStringResult   callback     // 回调
    );
};


} // namespace land