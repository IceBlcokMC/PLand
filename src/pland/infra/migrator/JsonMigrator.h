#pragma once
#include "pland/Global.h"

#include <functional>

#include <ll/api/Expected.h>

#include <mc/deps/core/utility/optional_ref.h>

#include <nlohmann/adl_serializer.hpp>


namespace land {

/**
 * @class JsonMigrator
 * @brief 数据迁移器，用于处理 JSON 数据的跨版本升级。
 *
 * 【行为规范 - 必读】：
 * 1. 版本号 N 代表的是“目标版本”。
 *    即：注册为版本 15 的 Executor，其职责是将数据从“小于 15 的任意状态”转换为“符合版本 15 规范的状态”。
 *
 * 2. 迁移路径是“跳跃式”的。
 *    如果当前数据是 v1，系统中有注册 [15, 26] 的迁移器：
 *    - 第一次迭代：直接寻找 > 1 的最小迁移器，找到 15。执行 v1 -> v15。
 *    - 第二次迭代：寻找 > 15 的最小迁移器，找到 26。执行 v15 -> v26。
 *    - 中间缺失的版本 (2-14, 16-25) 会被自动跳过。
 */
class JsonMigrator {
public:
    using Version  = int32_t;
    using Executor = std::function<ll::Expected<>(nlohmann::json& data)>;

    LDAPI JsonMigrator();
    LDAPI virtual ~JsonMigrator();
    LD_DISABLE_COPY_AND_MOVE(JsonMigrator);

    /**
     * @brief 注册迁移器
     * @param version 目标版本号 (执行后数据将变为此版本)
     * @param executor 迁移逻辑
     * @param cover 是否覆盖已存在的版本
     */
    LDAPI void registerMigrator(Version version, Executor executor, bool cover = false);

    /**
     * @brief 批量注册范围迁移器（通常用于处理连续的、逻辑相同的中间版本）
     */
    LDAPI void registerRangeMigrator(Version from, Version to, Executor executor, bool cover = false);

    LDNDAPI optional_ref<const Executor> getExecutor(Version version) const;

    enum class MigrateResult : uint8_t {
        Success,                     // 迁移成功
        SkipByNoAvailableMigrate,    // 没有可用的迁移器，跳过
        SkipByCurrentVersionTooHigh, // 当前版本高于目标版本，跳过
    };

    /**
     * @brief 核心迁移函数
     * @param data 待迁移的 JSON 对象
     * @param targetVersion 最终要达到的目标版本
     * @param allowVersionGap 是否允许版本断层。
     *        - 若为 true：从 v1 迁移到 v15 时，即使中间没有 2-14 的迁移器也会继续。
     *        - 若为 false：要求每一步版本提升必须连续 (N -> N+1)。
     */
    LDNDAPI virtual ll::Expected<MigrateResult>
    migrate(nlohmann::json& data, Version targetVersion, bool allowVersionGap = true) const;

    LDNDAPI std::optional<Version> getMinVersion() const;

    LDNDAPI std::optional<Version> getMaxVersion() const;

    inline static constexpr std::string_view VersionKey = "version";

protected:
    struct Route {
        const char* src;
        const char* dst;
    };

    /**
     * @brief 将JSON数据中的路径从src映射到dst
     *
     * @param root JSON数据的引用，将被修改以包含路径映射信息
     * @param src 源路径字符串视图，表示原始路径
     * @param dst 目标路径字符串视图，表示映射后的目标路径
     */
    static void mapPath(nlohmann::json& root, std::string_view src, std::string_view dst);

    static void mapPath(nlohmann::json& root, Route const& route);

    std::map<Version, Executor> mMigrators_;
};

} // namespace land
