#pragma once
#include <string>

namespace land {


enum class EconomyBackend { LegacyMoney, ScoreBoard };

using EconomyKit [[deprecated("Use EconomyBackend instead")]] = EconomyBackend;

struct EconomyConfig {
    bool           enabled        = false;
    EconomyBackend backend        = EconomyBackend::LegacyMoney;
    std::string    scoreboardName = "Scoreboard";
    std::string    currencyName   = "Coin";
};


} // namespace land