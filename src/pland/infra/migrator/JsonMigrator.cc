#include "JsonMigrator.h"

#include <nlohmann/json.hpp>

namespace land {


JsonMigrator::JsonMigrator()  = default;
JsonMigrator::~JsonMigrator() = default;

void JsonMigrator::registerMigrator(Version version, Executor executor, bool cover) {
    if (cover) {
        mMigrators_.insert_or_assign(version, executor);
    } else if (!mMigrators_.contains(version)) {
        mMigrators_.emplace(version, std::move(executor));
    }
}

void JsonMigrator::registerRangeMigrator(Version from, Version to, Executor executor, bool cover) {
    for (Version v = from; v <= to; ++v) {
        registerMigrator(v, executor, cover);
    }
}

optional_ref<JsonMigrator::Executor const> JsonMigrator::getExecutor(Version version) const {
    if (auto it = mMigrators_.find(version); it != mMigrators_.end()) {
        return it->second;
    }
    return {};
}

ll::Expected<JsonMigrator::MigrateResult>
JsonMigrator::migrate(nlohmann::json& data, Version targetVersion, bool allowVersionGap) const {
    if (mMigrators_.empty()) return MigrateResult::SkipByNoAvailableMigrate;

    // 获取当前版本，如果 JSON 中没有版本字段，默认视为 0
    Version currentVersion = data.value(VersionKey, Version(0));

    // 如果已经达到或超过目标版本，直接返回
    if (currentVersion >= targetVersion) {
        return MigrateResult::SkipByCurrentVersionTooHigh;
    }

    // 循环寻找“下一个可用的迁移器”
    // 使用 std::map::upper_bound(V) 寻找 key > V 的第一个元素
    auto it = mMigrators_.upper_bound(currentVersion);

    while (it != mMigrators_.end() && it->first <= targetVersion) {
        Version nextRegisteredVersion = it->first;
        auto&   executor              = it->second;

        // 校验版本连续性
        if (!allowVersionGap && nextRegisteredVersion != currentVersion + 1) {
            return ll::makeStringError(
                fmt::format(
                    "Data migration gap detected: current v{}, next available v{}. But gap is not allowed.",
                    currentVersion,
                    nextRegisteredVersion
                )
            );
        }

        // 执行具体的迁移函数
        try {
            if (auto res = std::invoke(executor, data); !res) {
                return ll::makeStringError(res.error().message()); // 迁移器内部返回错误，中止迁移
            }
        } catch (std::exception const& e) {
            return ll::makeStringError(
                fmt::format("Exception during migration to v{}: {}", nextRegisteredVersion, e.what())
            );
        }

        // 更新当前版本号到数据中
        currentVersion   = nextRegisteredVersion;
        data[VersionKey] = currentVersion;

        // 继续寻找下一个大于当前版本的迁移器
        it = mMigrators_.upper_bound(currentVersion);
    }

    if (!allowVersionGap && currentVersion < targetVersion) {
        return ll::makeStringError(fmt::format("Migration failed to reach target v{}", targetVersion));
    }

    return MigrateResult::Success;
}

std::optional<JsonMigrator::Version> JsonMigrator::getMinVersion() const {
    if (mMigrators_.empty()) {
        return std::nullopt;
    }
    return mMigrators_.begin()->first;
}
std::optional<JsonMigrator::Version> JsonMigrator::getMaxVersion() const {
    if (mMigrators_.empty()) {
        return std::nullopt;
    }
    return mMigrators_.rbegin()->first;
}
void JsonMigrator::mapPath(nlohmann::json& root, std::string_view src, std::string_view dst) {
    // 辅助函数：把 "land.bought.xxx" 变成 "/land/bought/xxx"
    auto toPtr = [](std::string_view s) {
        std::string res = "/";
        for (char c : s) {
            res += (c == '.' ? '/' : c);
        }
        return nlohmann::json::json_pointer(res);
    };

    auto srcPtr = toPtr(src);
    if (root.contains(srcPtr)) {
        // std::move 转移数据到新家（自动创建缺失的父节点）
        root[toPtr(dst)] = std::move(root[srcPtr]);

        // 擦除旧数据的数据痕迹
        auto parentPtr = srcPtr.parent_pointer();
        if (root.contains(parentPtr)) {
            root[parentPtr].erase(srcPtr.back());
        }
    }
}
void JsonMigrator::mapPath(nlohmann::json& root, Route const& route) { mapPath(root, route.src, route.dst); }

} // namespace land