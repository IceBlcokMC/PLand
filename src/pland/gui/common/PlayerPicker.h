#pragma once
#include "pland/Global.h"

#include "ll/api/service/PlayerInfo.h"

#include <functional>

class Player;
namespace mce {
class UUID;
}

namespace land::gui {

struct PlayerPickerRouter {
    PlayerPickerRouter() = delete;

    using OfflinePlayerInfo = ll::service::PlayerInfo::PlayerInfoEntry;

    using result_t   = std::variant<Player*, OfflinePlayerInfo>;
    using callback_t = std::function<void(Player& self, result_t res)>;
    using back_t     = std::function<void(Player&)>;

    LDAPI static void sendTo(
        Player&           player,
        callback_t const& callback,
        back_t const&     backTo                  = nullptr,
        bool              includeSimulatedPlayers = false
    );
};

struct OnlinePlayerPicker {
    OnlinePlayerPicker() = delete;

    LDAPI static void sendTo(
        Player&                               player,
        PlayerPickerRouter::callback_t const& callback,
        PlayerPickerRouter::back_t            backTo                  = nullptr,
        bool                                  includeSimulatedPlayers = false
    );
};

struct OfflinePlayerPicker {
    OfflinePlayerPicker() = delete;

    LDAPI static void sendTo(
        Player&                               player,
        PlayerPickerRouter::callback_t const& callback,
        const std::string&                    defaultVal = "",
        const std::string&                    errorText  = "",
        int                                   retryCount = 0,
        int                                   maxRetries = 10
    );
};


} // namespace land::gui