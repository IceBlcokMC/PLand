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

    using EditStringResult = std::function<void(Player& self, string result)>;
    LDAPI static void sendTo(
        Player&          player,
        string const&    title,       // 标题
        string const&    text,        // 提示
        string const&    defaultValu, // 默认值
        EditStringResult callback     // 回调
    );
};


} // namespace land