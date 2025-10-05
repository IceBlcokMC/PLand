#include "Telemetry.h"
#include "bstats/Bukkit.h"
#include "ll/api/utils/SystemUtils.h"
#include "pland/PLand.h"
#include "pland/Version.h"
#include "pland/infra/Config.h"

#include "ll/api/Versions.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/coro/InterruptableSleep.h"
#include "ll/api/io/FileUtils.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/thread/ThreadPoolExecutor.h"
#include "ll/api/utils/RandomUtils.h"


#include "mc/common/BuildInfo.h"
#include "mc/common/Common.h"
#include "mc/platform/UUID.h"
#include "mc/server/PropertiesSettings.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"


#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"

#include "cpr/api.h"
#include "cpr/body.h"
#include "cpr/cpr.h"
#include "cpr/cprtypes.h"

#include "magic_enum/magic_enum.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>


namespace land::network {


struct Telemetry::Impl {
    bstats::Bukkit                                mBody;
    std::shared_ptr<std::atomic<bool>>            mQuit;
    std::shared_ptr<ll::coro::InterruptableSleep> mSleep;

    std::string const& getServiceUUID(PLand* pland) const {
        static std::optional<std::string> uuid;
        if (!uuid) {
            auto path = pland->getSelf().getDataDir() / u8"telemetry_uuid";
            uuid      = ll::file_utils::readFile(path).or_else([&] {
                auto temp = mce::UUID::random().asString();
                ll::file_utils::writeFile(path, temp);
                return std::optional{std::move(temp)};
            });
        }
        return uuid.value();
    }

    void collect() {
        mBody.playerAmount =
            ll::service::getLevel().transform([](auto& level) { return level.getActivePlayerCount(); }).value_or(0);

        {
            std::unordered_map<std::string, int> platforms;
            ll::service::getLevel().transform([&platforms](auto& level) {
                level.forEachPlayer([&platforms](Player& player) {
                    std::string platformName = std::string{magic_enum::enum_name(player.mBuildPlatform)};
                    if (platforms.find(platformName) == platforms.end()) {
                        platforms.emplace(platformName, 1);
                    } else {
                        platforms[platformName] += 1;
                    }
                    return true;
                });
                return true;
            });

            auto iter =
                std::find_if(mBody.service.customCharts.begin(), mBody.service.customCharts.end(), [](auto& chart) {
                    if (std::holds_alternative<bstats::AdvancedPie>(chart)) {
                        auto& pie = std::get<bstats::AdvancedPie>(chart);
                        return pie.chartId == "player_platform";
                    }
                    return false;
                });
            if (iter == mBody.service.customCharts.end()) {
                mBody.service.customCharts.emplace_back(bstats::AdvancedPie{"player_platform", {std::move(platforms)}});
                return;
            }
            auto& pie       = std::get<bstats::AdvancedPie>(*iter);
            pie.data.values = std::move(platforms); // update
        }
    }

    void submit() {
        try {
            collect();

            cpr::Post(
                cpr::Url{
                    bstats::Bukkit::PostUrl
            },
                cpr::Body{mBody.to_json().dump()},
                cpr::Header{
                    {"Accept", bstats::AcceptHeader},
                    {"Content-Type", bstats::ContentTypeHeader},
                    {"User-Agent", bstats::UserAgentHeader}
                }
            );
#ifdef DEBUG
            PLand::getInstance().getSelf().getLogger().debug("Telemetry: submitted");
#endif
        } catch (...) {
#ifdef DEBUG
            PLand::getInstance().getSelf().getLogger().error("Telemetry: failed to submit");
#endif
        }
    }

    void launch(PLand* pland) {
        mQuit  = std::make_shared<std::atomic<bool>>(false);
        mSleep = std::make_shared<ll::coro::InterruptableSleep>();
        ll::coro::keepThis([sleep = mSleep, quit = mQuit, this]() -> ll::coro::CoroTask<> {
            co_await sleep->sleepFor(std::chrono::minutes{1} * ll::random_utils::rand(3.0, 6.0));
            if (!quit->load()) {
                submit();
            }
            co_await sleep->sleepFor(std::chrono::minutes{1} * ll::random_utils::rand(1.0, 30.0));
            if (!quit->load()) {
                submit();
            }
            while (!quit->load()) {
                co_await sleep->sleepFor(std::chrono::minutes{30});
                if (quit->load()) {
                    break;
                }
                submit();
            }
            co_return;
        }).launch(*pland->getThreadPool());
    }

    void stop() const {
        mQuit->store(true);
        mSleep->interrupt(true);
    }

    Impl(PLand* pland) {
        mBody = bstats::Bukkit(getServiceUUID(pland), 27389);

        mBody.osArch                = "amd64";
        mBody.coreCount             = std::thread::hardware_concurrency();
        mBody.onlineMode            = ll::service::getPropertiesSettings().value().mIsOnlineMode;
        mBody.service.pluginVersion = PLAND_VERSION_STRING;

        mBody.osName    = ll::sys_utils::isWine() ? "Linux" : "Windows";
        mBody.osVersion = ll::sys_utils::getSystemVersion().to_string();

        mBody.bukkitVersion = Common::getBuildInfo().mBuildId;

        // custom
        mBody.service.customCharts.emplace_back(
            bstats::SimplePie{"levilamina_version", {ll::getLoaderVersion().to_string()}}
        );

        launch(pland);
    }
    ~Impl() { stop(); }
};

Telemetry::Telemetry(PLand* pland) : mImpl(nullptr) {
    auto const& enable = Config::cfg.internal.telemetry;
    if (enable) {
        mImpl = std::make_unique<Impl>(pland);
    }
}
Telemetry::~Telemetry() = default;


} // namespace land::network
