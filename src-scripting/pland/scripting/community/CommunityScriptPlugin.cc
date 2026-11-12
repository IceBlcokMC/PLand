#include "CommunityScriptPlugin.h"

#include "pland/scripting/EngineHelper.h"
#include "pland/scripting/pch.h"

namespace land::scripting {

CommunityScriptPlugin::CommunityScriptPlugin(ll::mod::Manifest manifest) : Mod(std::move(manifest)) {}

CommunityScriptPlugin::~CommunityScriptPlugin() { release(); }

std::shared_ptr<ll::mod::Mod> CommunityScriptPlugin::current() {
    auto data = jspp::EngineScope::currentEngineChecked().getData<EngineOwnData>();
    if (data->isPrivateEngine) {
        return nullptr; // NativeMod
    }
    return data->pluginInst;
}

} // namespace land::scripting