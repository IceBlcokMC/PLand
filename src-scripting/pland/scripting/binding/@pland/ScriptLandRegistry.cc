#include "pland/scripting/binding/Helper.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "pland/PLand.h"
#include "pland/land/repo/LandRegistry.h"

namespace land::scripting {

decltype(Modules::kScriptLandRegistry) Modules::kScriptLandRegistry =
    jspp::binding::defClass<void>("LandRegistry")
        .instanceProvider([]() -> auto& { return PLand::getInstance().getLandRegistry(); })
        .func("isAdmin", &LandRegistry::isOperator)
        .func("addAdmin", &LandRegistry::addOperator)
        .func("removeAdmin", &LandRegistry::removeOperator)
        .func("getAdmins", &LandRegistry::getOperators)
        .build();

} // namespace land::scripting