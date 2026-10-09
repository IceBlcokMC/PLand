#pragma once

#include "pland/scripting/pch.h"

namespace land::scripting {


struct Modules {
    static void bind(jspp::Engine& engine);

    [[nodiscard]] static jspp::ModuleMeta const& getRuntimeModule();
    [[nodiscard]] static jspp::ModuleMeta const& getMinecraftModule();
    [[nodiscard]] static jspp::ModuleMeta const& getLeviLaminaModule();
    [[nodiscard]] static jspp::ModuleMeta const& getPLandModule();

    // @minecraft
    static jspp::ClassMeta const kScriptPlayer;
    static jspp::EnumMeta const  kScriptCommandPermissionLevel;
    static jspp::EnumMeta const  kScriptCommandParameterOption;
    static jspp::ClassMeta const kScriptCommandOrigin;
    static jspp::ClassMeta const kScriptCommandOutput;
    static jspp::EnumMeta const  kScriptCommandOriginType;
    static jspp::EnumMeta const  kScriptModalFormCancelReason;

    // @levilamina
    static jspp::ClassMeta const kScriptCommandRegistrar;
    static jspp::ClassMeta const kScriptCommandHandle;
    static jspp::ClassMeta const kScriptRuntimeOverload;
    static jspp::EnumMeta const  kScriptCommandParamKind;
    static jspp::ClassMeta const kScriptCustomForm;
    static jspp::ClassMeta const kScriptModalForm;
    static jspp::ClassMeta const kScriptSimpleForm;
    static jspp::EnumMeta const  kScriptModalFormSelectedButton;
};


} // namespace land::scripting