#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "pland/economy/EconomySystem.h"

namespace land::scripting {

decltype(Modules::kScriptEconomy) Modules::kScriptEconomy = //
    jspp::binding::defClass<void>("economy")
        .var_readonly("isEnabled", EconomySystem::isEnabled)
        .var_readonly("currencyName", EconomySystem::getCurrencyName())

        .instanceProvider([]() -> econbridge::IEconomy& { return *EconomySystem::getInstance().get(); })
        .func("get", &econbridge::IEconomy::get)
        .func("set", &econbridge::IEconomy::set)
        .func("add", &econbridge::IEconomy::add)
        .func("reduce", &econbridge::IEconomy::reduce)
        .func("transfer", &econbridge::IEconomy::transfer)
        .build();

} // namespace land::scripting