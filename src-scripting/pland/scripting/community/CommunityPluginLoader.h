#pragma once
#include "ll/api/mod/ModManager.h"

#include "pland/scripting/pch.h"

namespace land::scripting {

class CommunityPluginLoader final : public ll::mod::ModManager {
    absl::flat_hash_map<std::string, std::unique_ptr<jspp::Engine>> mEngines{};

public:
    explicit CommunityPluginLoader();
    ~CommunityPluginLoader() override;

    [[nodiscard]] jspp::Engine* getEngine(std::string_view name) const;

protected:
    ll::Expected<> load(ll::mod::Manifest manifest) override;
    ll::Expected<> unload(std::string_view name) override;
};

} // namespace land::scripting
