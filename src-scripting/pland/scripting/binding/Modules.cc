#include "Modules.h"

namespace land::scripting {


void Modules::bind(jspp::Engine& engine) {
    engine.registerModule(getRuntimeModule());
    engine.registerModule(getMinecraftModule());
    engine.registerModule(getLeviLaminaModule());
    engine.registerModule(getPLandModule());
}

jspp::ModuleMeta const& Modules::getMinecraftModule() {
    static jspp::ModuleMeta m = jspp::binding::defModule("@minecraft") //
                                    .export_class(kScriptPlayer)
                                    .export_enum(kScriptCommandPermissionLevel)
                                    .export_enum(kScriptCommandParameterOption)
                                    .export_class(kScriptCommandOrigin)
                                    .export_class(kScriptCommandOutput)
                                    .export_enum(kScriptCommandOriginType)
                                    .export_enum(kScriptModalFormCancelReason)
                                    .build();
    return m;
}
jspp::ModuleMeta const& Modules::getLeviLaminaModule() {
    static jspp::ModuleMeta m = jspp::binding::defModule("@levilamina") //
                                    .export_class(kScriptCommandRegistrar)
                                    .export_class(kScriptCommandHandle)
                                    .export_class(kScriptRuntimeOverload)
                                    .export_enum(kScriptCommandParamKind)
                                    .export_class(kScriptCustomForm)
                                    .export_class(kScriptModalForm)
                                    .export_class(kScriptSimpleForm)
                                    .export_enum(kScriptModalFormSelectedButton)
                                    .export_class(kScriptPlayerInfo)
                                    .build();
    return m;
}
jspp::ModuleMeta const& Modules::getPLandModule() {
    static jspp::ModuleMeta m = jspp::binding::defModule("@pland").build();
    return m;
}

} // namespace land::scripting