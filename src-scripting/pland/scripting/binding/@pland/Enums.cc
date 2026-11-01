#include "pland/scripting/binding/Helper.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"


#include "pland/enums/LandOwnershipKind.h"
decltype(land::scripting::Modules::kScriptLandOwnershipKind) land::scripting::Modules::kScriptLandOwnershipKind = //
    helper::auto_gen_enum_def<LandOwnershipKind>("LandOwnershipKind");

#include "pland/enums/LandHoldType.h"
decltype(land::scripting::Modules::kScriptLandHoldType) land::scripting::Modules::kScriptLandHoldType = //
    helper::auto_gen_enum_def<LandHoldType>("LandHoldType");

#include "pland/enums/LeaseState.h"
decltype(land::scripting::Modules::kScriptLeaseState) land::scripting::Modules::kScriptLeaseState = //
    helper::auto_gen_enum_def<LeaseState>("LeaseState");

#include "pland/enums/LandType.h"
decltype(land::scripting::Modules::kScriptLandType) land::scripting::Modules::kScriptLandType = //
    helper::auto_gen_enum_def<LandType>("LandType");

#include "pland/enums/LandRole.h"
decltype(land::scripting::Modules::kScriptLandRole) land::scripting::Modules::kScriptLandRole = //
    helper::auto_gen_enum_def<LandRole>("LandRole");
