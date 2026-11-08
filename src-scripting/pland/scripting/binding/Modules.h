#pragma once

#include "pland/scripting/pch.h"
namespace land::scripting {


struct Modules {
    static void bind(jspp::Engine& engine);

    [[nodiscard]] static jspp::ModuleMeta const& getRuntimeModule();
    [[nodiscard]] static jspp::ModuleMeta const& getMinecraftModule();
    [[nodiscard]] static jspp::ModuleMeta const& getLeviLaminaModule();
    [[nodiscard]] static jspp::ModuleMeta const& getPLandModule();

    // @runtime
    static jspp::ClassMeta const kScriptFileSystem; // fs
    static jspp::ClassMeta const kScriptPath;       // path
    static jspp::ClassMeta const kScriptBridge;

    // @minecraft
    static jspp::ClassMeta const kScriptPlayer;
    static jspp::EnumMeta const  kScriptCommandPermissionLevel;
    static jspp::EnumMeta const  kScriptCommandParameterOption;
    static jspp::ClassMeta const kScriptCommandOrigin;
    static jspp::ClassMeta const kScriptCommandOutput;
    static jspp::EnumMeta const  kScriptCommandOriginType;
    static jspp::EnumMeta const  kScriptModalFormCancelReason;
    static jspp::EnumMeta const  kScriptAbilitiesIndex;

    // @levilamina
    static jspp::ClassMeta const kScriptCommandRegistrar;
    static jspp::ClassMeta const kScriptCommandHandle;
    static jspp::ClassMeta const kScriptRuntimeOverload;
    static jspp::EnumMeta const  kScriptCommandParamKind;
    static jspp::ClassMeta const kScriptCustomForm;
    static jspp::ClassMeta const kScriptModalForm;
    static jspp::ClassMeta const kScriptSimpleForm;
    static jspp::EnumMeta const  kScriptModalFormSelectedButton;
    static jspp::ClassMeta const kScriptPlayerInfo;
    static jspp::ClassMeta const kScriptLogger;
    static jspp::EnumMeta const  kScriptLogLevel;
    static jspp::ClassMeta const kScriptI18n;
    static jspp::ClassMeta const kScriptEventBus;      // impl in ScriptEventImpl.cc
    static jspp::EnumMeta const  kScriptEventPriority; // impl in ScriptEventImpl.cc
    static jspp::ClassMeta const kScriptEvent;         // impl in ScriptEventImpl.cc

    // @pland
    static jspp::ClassMeta const kScriptLandRegistry;
    static jspp::ClassMeta const kScriptAABB;
    static jspp::ClassMeta const kScriptLand;
    static jspp::EnumMeta const  kScriptLandOwnershipKind;
    static jspp::EnumMeta const  kScriptLandHoldType;
    static jspp::EnumMeta const  kScriptLeaseState;
    static jspp::EnumMeta const  kScriptLandType;
    static jspp::EnumMeta const  kScriptLandRole;
    static jspp::ClassMeta const kScriptDrawer;
    static jspp::ClassMeta const kScriptEconomy;
    static jspp::EnumMeta const  kScriptLandRecycleReason;
    static jspp::ClassMeta const kScriptConfigReloadEvent;         // impl in ScriptEventImpl.cc
    static jspp::ClassMeta const kScriptLandRecycleEvent;          // impl in ScriptEventImpl.cc
    static jspp::ClassMeta const kScriptLandResizedEvent;          // impl in ScriptEventImpl.cc
    static jspp::ClassMeta const kScriptLandStateChangedEvent;     // impl in ScriptEventImpl.cc
    static jspp::ClassMeta const kScriptMemberChangedEvent;        // impl in ScriptEventImpl.cc
    static jspp::ClassMeta const kScriptMembersClearedEvent;       // impl in ScriptEventImpl.cc
    static jspp::ClassMeta const kScriptOwnerChangedEvent;         // impl in ScriptEventImpl.cc
    static jspp::ClassMeta const kScriptLandOwnershipChangedEvent; // impl in ScriptEventImpl.cc
};


} // namespace land::scripting