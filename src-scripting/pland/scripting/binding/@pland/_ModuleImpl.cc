#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "pland/Global.h"

namespace land::scripting {

jspp::ModuleMeta const& Modules::getPLandModule() {
    static jspp::ModuleMeta m = jspp::binding::defModule("@pland") //
                                    .export_constant("INVALID_LAND_ID", INVALID_LAND_ID)
                                    .export_constant("SYSTEM_ACCOUNT_UUID_STR", SYSTEM_ACCOUNT_UUID_STR)
                                    .export_class(kScriptLandRegistry)
                                    .export_class(kScriptAABB)
                                    .export_class(kScriptLand)
                                    .export_enum(kScriptLandOwnershipKind)
                                    .export_enum(kScriptLandHoldType)
                                    .export_enum(kScriptLeaseState)
                                    .export_enum(kScriptLandType)
                                    .export_enum(kScriptLandRole)
                                    .export_class(kScriptDrawer)
                                    .export_class(kScriptEconomy)
                                    .export_enum(kScriptLandRecycleReason)
                                    .export_class(kScriptConfigReloadEvent)
                                    .export_class(kScriptLandRecycleEvent)
                                    .export_class(kScriptLandResizedEvent)
                                    .export_class(kScriptLandStateChangedEvent)
                                    .export_class(kScriptMemberChangedEvent)
                                    .export_class(kScriptMembersClearedEvent)
                                    .export_class(kScriptOwnerChangedEvent)
                                    .build();
    return m;
}

} // namespace land::scripting