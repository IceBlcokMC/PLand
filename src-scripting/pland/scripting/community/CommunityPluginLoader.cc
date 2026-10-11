#include "CommunityPluginLoader.h"

#include "CommunityScriptPlugin.h"
#include "pland/PLand.h"
#include "pland/scripting/EngineHelper.h"
#include "pland/scripting/EngineHelper.h"
#include "pland/scripting/ScriptingBootstrap.h"

#include "ll/api/service/GamingStatus.h"

namespace land::scripting {


constexpr std::string_view kScriptPluginType = "pland-script";

CommunityPluginLoader::CommunityPluginLoader() : ModManager(kScriptPluginType) {}
CommunityPluginLoader::~CommunityPluginLoader() = default;

jspp::Engine* CommunityPluginLoader::getEngine(std::string_view name) const {
    auto iter = mEngines.find(name);
    if (iter == mEngines.end()) {
        return nullptr;
    }
    return iter->second.get();
}

ll::Expected<> CommunityPluginLoader::load(ll::mod::Manifest manifest) {
    if (hasMod(manifest.name)) {
        return ll::makeStringError("Plugin has already loaded");
    }
    auto engine = EngineHelper::newEngine();
    auto plugin = std::make_shared<CommunityScriptPlugin>(manifest);

    {
        auto ownData                   = engine->getData<EngineOwnData>();
        ownData->communityScriptPlugin = plugin;
        ownData->communityPluginName   = manifest.name;
    }

    auto _ = jspp::EngineScope{*engine};
    try {
        auto entry = plugin->getModDir() / manifest.entry;

        if (auto ok = EngineHelper::loadScript(entry, *engine); !ok) {
            return ll::forwardError(ok.error());
        }

        {
            plugin->onLoad([this](ll::mod::Mod& self) -> bool {
                auto& name = self.getName();
                if (auto engine = this->getEngine(name)) {
                    auto expected =
                        EngineHelper::callScriptLifecycleHook(*engine, EngineHelper::LifecycleHookKind::kOnLoad);
                    if (!expected) {
                        expected.error().log(PLand::getInstance().getSelf().getLogger());
                        return false;
                    }
                }
                return true;
            });
            plugin->onEnable([this](ll::mod::Mod& self) -> bool {
                auto& name = self.getName();
                if (auto engine = this->getEngine(name)) {
                    auto expected =
                        EngineHelper::callScriptLifecycleHook(*engine, EngineHelper::LifecycleHookKind::kOnEnable);
                    if (!expected) {
                        expected.error().log(PLand::getInstance().getSelf().getLogger());
                        return false;
                    }
                }
                return true;
            });
            plugin->onDisable([this](ll::mod::Mod& self) -> bool {
                auto& name = self.getName();
                if (auto engine = this->getEngine(name)) {
                    auto expected =
                        EngineHelper::callScriptLifecycleHook(*engine, EngineHelper::LifecycleHookKind::kOnDisable);
                    if (!expected) {
                        expected.error().log(PLand::getInstance().getSelf().getLogger());
                        return false;
                    }
                }
                return true;
            });
        }

        mEngines.emplace(manifest.name, std::move(engine));
        return plugin->onLoad()
            .transform([this, manifest, plugin] mutable { addMod(manifest.name, plugin); })
            .and_then([plugin]() -> ll::Expected<> {
                if (ll::getGamingStatus() == ll::GamingStatus::Running) {
                    return plugin->onEnable(); // hot load
                }
                return {}; // staring
            })
            .or_else([manifest, this](ll::Error err [[maybe_unused]]) -> ll::Expected<> {
                return this->unload(manifest.name); // cleanup
            });
    } catch (jspp::Exception const& exception) {
        mEngines.erase(manifest.name);
        return ll::makeStringError(
            fmt::format(
                "failed to load plugin '{}', error: {}\n{}",
                manifest.name,
                exception.what(),
                exception.stacktrace()
            )
        );
    }
}

ll::Expected<> CommunityPluginLoader::unload(std::string_view name) {
    auto engine = getEngine(name);
    if (!engine) {
        return ll::makeStringError(fmt::format("plugin '{}' does not exist", name));
    }

    if (auto res = std::static_pointer_cast<CommunityScriptPlugin>(getMod(name))->onUnload(); !res) {
        return res;
    }

    mEngines.erase(name);
    eraseMod(name);
    return {};
}

} // namespace land::scripting