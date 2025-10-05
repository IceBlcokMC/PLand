#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"

namespace bstats {


struct SimplePie {
    std::string chartId;
    struct {
        std::variant<std::string, int> value; // Value
    } data;

    explicit SimplePie(std::string_view chartId, std::variant<std::string, int> value)
    : chartId(chartId),
      data{std::move(value)} {}

    inline nlohmann::json to_json() const {
        nlohmann::json pie;
        pie["chartId"] = chartId;
        if (std::holds_alternative<std::string>(data.value)) {
            pie["data"]["value"] = std::get<std::string>(data.value);
        } else {
            pie["data"]["value"] = std::get<int>(data.value);
        }
        return pie;
    }
};

struct AdvancedPie {
    std::string chartId;
    struct {
        std::unordered_map<std::string, int> values;
    } data;

    explicit AdvancedPie(std::string_view chartId, std::unordered_map<std::string, int> values)
    : chartId(chartId),
      data{std::move(values)} {}

    inline nlohmann::json to_json() const {
        nlohmann::json pie;
        pie["chartId"]        = chartId;
        pie["data"]["values"] = nlohmann::json::object();

        auto& values = pie["data"]["values"];
        for (const auto& [key, value] : data.values) {
            values[key] = value;
        }
        return pie;
    }
};

using SingleLine = SimplePie; // data definition is the same

struct Bukkit {
    std::string serverUUID; // UUID of the server (required)
    struct Service {
        int                                               id;           // plugin id (required)
        std::vector<std::variant<SimplePie, AdvancedPie>> customCharts; // Custom charts
        std::optional<std::string> pluginVersion{};                     // Plugin version (mapping to 'pluginVersion')
    } service;

    std::optional<std::string> osArch;       // Operating system architecture (mapping to 'osArch')
    std::optional<int>         coreCount;    // Number of CPU cores (mapping to 'coreCount')
    std::optional<int>         onlineMode;   // 1: online, 0: offline (mapping to 'onlineMode')
    std::optional<int>         playerAmount; // Number of online players (mapping to 'players')


    std::optional<std::string> bukkitVersion; // Bukkit version (mapping to 'minecraftVersion')

    // Special fields, parsed to OS
    // (due to the special behavior of the bstats backend, these two fields need to exist together)
    std::optional<std::string> osName;    // Operating system name (Hard coding)
    std::optional<std::string> osVersion; // Operating system version

    // Useless field (relative to Bedrock)
    [[deprecated]] std::optional<std::string> bukkitName;  // Bukkit name (Hard coding) (mapping to 'serverSoftware')
    [[deprecated]] std::optional<std::string> javaVersion; // Java version

    Bukkit() = default;
    explicit Bukkit(std::string_view serverUUID, int pluginId) : serverUUID(serverUUID), service{pluginId, {}} {}

    inline nlohmann::json to_json() const {
        nlohmann::json body;
        body["serverUUID"]              = serverUUID;
        body["service"]                 = nlohmann::json::object();
        body["service"]["id"]           = service.id;
        body["service"]["customCharts"] = nlohmann::json::array();
        for (const auto& chart : service.customCharts) {
            body["service"]["customCharts"].push_back(
                std::visit([](const auto& chart) { return chart.to_json(); }, chart)
            );
        }
        if (service.pluginVersion.has_value()) {
            body["service"]["pluginVersion"] = service.pluginVersion.value();
        }
        if (osName.has_value()) {
            body["osName"] = osName.value();
        }
        if (osArch.has_value()) {
            body["osArch"] = osArch.value();
        }
        if (osVersion.has_value()) {
            body["osVersion"] = osVersion.value();
        }
        if (coreCount.has_value()) {
            body["coreCount"] = coreCount.value();
        }
        if (javaVersion.has_value()) {
            body["javaVersion"] = javaVersion.value();
        }
        if (bukkitVersion.has_value()) {
            body["bukkitVersion"] = bukkitVersion.value();
        }
        if (bukkitName.has_value()) {
            body["bukkitName"] = bukkitName.value();
        }
        if (onlineMode.has_value()) {
            body["onlineMode"] = onlineMode.value();
        }
        if (playerAmount.has_value()) {
            body["playerAmount"] = playerAmount.value();
        }
        return body;
    }


    // default charts keywords
    inline static constexpr std::string_view ServersChartId          = "servers";
    inline static constexpr std::string_view PlayersChartId          = "players";
    inline static constexpr std::string_view OnlineModeChartId       = "onlineMode";
    inline static constexpr std::string_view MinecraftVersionChartId = "minecraftVersion";
    inline static constexpr std::string_view ServerSoftwareChartId   = "serverSoftware";
    inline static constexpr std::string_view PluginVersionChartId    = "pluginVersion";
    inline static constexpr std::string_view CoreCountChartId        = "coreCount";
    inline static constexpr std::string_view OsArchChartId           = "osArch";
    inline static constexpr std::string_view OsChartId               = "os";
    inline static constexpr std::string_view LocationChartId         = "location";
    inline static constexpr std::string_view JavaVersionChartId      = "javaVersion";
    inline static constexpr std::string_view LocationMapChartId      = "locationMap";

    // bStats api url
    inline static constexpr std::string_view PostUrl = "https://bstats.org/api/v2/data/bukkit";
};


// Headers
inline static constexpr auto AcceptHeader      = "application/json";
inline static constexpr auto ContentTypeHeader = "application/json";
inline static constexpr auto UserAgentHeader   = "Metrics-Service/1";


} // namespace bstats