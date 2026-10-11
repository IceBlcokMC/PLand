#pragma once
#include "ll/api/mod/Mod.h"

namespace land::scripting {

class CommunityScriptPlugin : public ll::mod::Mod {
    friend class CommunityPluginLoader;

public:
    explicit CommunityScriptPlugin(ll::mod::Manifest manifest);
    ~CommunityScriptPlugin();

    static std::shared_ptr<ll::mod::Mod> current();
};

} // namespace land::scripting