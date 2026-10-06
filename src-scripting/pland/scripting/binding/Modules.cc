#include "Modules.h"

namespace land::scripting {


void Modules::bind(jspp::Engine& engine) {
    engine.registerModule(getUtilsModule());
    engine.registerModule(getMinecraftModule());
    engine.registerModule(getLeviLaminaModule());
    engine.registerModule(getPLandModule());
}

jspp::ModuleMeta const& Modules::getMinecraftModule() {
    static jspp::ModuleMeta m = jspp::binding::defModule("@minecraft").build();
    return m;
}
jspp::ModuleMeta const& Modules::getLeviLaminaModule() {
    static jspp::ModuleMeta m = jspp::binding::defModule("@levilamina").build();
    return m;
}
jspp::ModuleMeta const& Modules::getPLandModule() {
    static jspp::ModuleMeta m = jspp::binding::defModule("@pland").build();
    return m;
}

} // namespace land::scripting