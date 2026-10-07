#pragma once

#include "pland/scripting/pch.h"

namespace land::scripting {


struct Modules {
    static void bind(jspp::Engine& engine);

    [[nodiscard]] static jspp::ModuleMeta const& getRuntimeModule();
    [[nodiscard]] static jspp::ModuleMeta const& getMinecraftModule();
    [[nodiscard]] static jspp::ModuleMeta const& getLeviLaminaModule();
    [[nodiscard]] static jspp::ModuleMeta const& getPLandModule();

    // minecraft module
    static jspp::ClassMeta const kScriptPlayer;
};


} // namespace land::scripting