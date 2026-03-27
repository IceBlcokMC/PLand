#pragma once
#include <chrono>

#include "fmt/chrono.h"
#include "fmt/format.h"
#include <fmt/core.h>

namespace land::time_utils {

/**
 * 获取当前时间的秒数（从Unix纪元开始计算）
 * @return 返回从1970年1月1日00:00:00 UTC到现在的秒数
 */
inline time_t nowSeconds() {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}

/**
 * 将从1970年1月1日00:00:00 UTC开始的秒数转换为系统时钟时间点
 * @param ts 自1970年1月1日00:00:00 UTC以来经过的秒数
 * @return 对应的系统时钟时间点
 */
inline std::chrono::system_clock::time_point toClockTime(time_t ts) {
    return std::chrono::system_clock::from_time_t(ts);
}


inline constexpr int HoursPerDay      = 24LL;                                            // 每天小时数
inline constexpr int MinutesPerHour   = 60LL;                                            // 每小时分钟数
inline constexpr int SecondsPerMinute = 60LL;                                            // 每分钟秒数
inline constexpr int SecondsPerDay    = HoursPerDay * MinutesPerHour * SecondsPerMinute; // 每天秒数

/**
 * 将天数转换为秒数
 * @param days 要转换的天数
 * @return 返回转换后的秒数
 */
inline time_t toSeconds(int days) noexcept { return static_cast<time_t>(days) * SecondsPerDay; }

/**
 * 将chrono系统时钟的时间点转换为自纪元以来的秒数
 * @param point chrono系统时钟的时间点
 * @return 返回自纪元以来的秒数
 */
inline time_t toSeconds(std::chrono::system_clock::time_point point) {
    return std::chrono::duration_cast<std::chrono::seconds>(point.time_since_epoch()).count();
}

/**
 * 将秒数向上取整为天数
 * @param seconds 要转换的秒数
 * @return 返回转换后的天数，如果输入小于等于0则返回0
 */
inline time_t ceilDays(time_t seconds) noexcept {
    if (seconds <= 0) return 0;
    return (seconds + SecondsPerDay - 1) / SecondsPerDay; // 向上取整
}

/**
 * 格式化时间点为指定格式的字符串
 * @param time 系统时钟时间点
 * @param localTime 是否使用本地时间，默认为true
 * @param fmt_str 时间格式字符串，默认为"%Y-%m-%d %H:%M:%S"
 * @return 格式化后的时间字符串
 */
inline std::string formatTime(
    std::chrono::system_clock::time_point time,
    bool                                  localTime = true,
    std::string_view                      fmt_str   = "%Y-%m-%d %H:%M:%S"
) {
    std::string format_pattern;
    format_pattern.reserve(fmt_str.size() + 3);
    format_pattern.append("{:").append(fmt_str).append("}");

    if (localTime) {
#if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
        std::chrono::zoned_time zt{std::chrono::current_zone(), time};
        return fmt::format(fmt::runtime(format_pattern), zt.get_local_time());
#else
        std::time_t t = std::chrono::system_clock::to_time_t(time);
        std::tm     tm_buf{};
#if defined(_WIN32)
        localtime_s(&tm_buf, &t);
#else
        localtime_r(&t, &tm_buf);
#endif
        return fmt::format(fmt::runtime(format_pattern), tm_buf);
#endif
    } else {
        return fmt::format(fmt::runtime(format_pattern), time);
    }
}


struct DurationParts {
    long long days{};    // 天数
    long long hours{};   // 小时数
    long long minutes{}; // 分钟数
    long long seconds{}; // 秒数
};
/**
 * @brief 将总秒数分解为天、小时、分钟和秒
 * @param totalSeconds 要分解的总秒数
 * @return DurationParts 包含分解后的天、小时、分钟和秒的结构体
 */
inline DurationParts decomposeDuration(time_t totalSeconds) {
    totalSeconds = std::max(totalSeconds, 0LL);

    long long days    = totalSeconds / 86400;
    long long hours   = (totalSeconds % 86400) / 3600;
    long long minutes = (totalSeconds % 3600) / 60;
    long long seconds = totalSeconds % 60;

    return {days, hours, minutes, seconds};
}

/**
 * 格式化持续时间为易读的字符串表示
 * @param durationSeconds 持续时间（秒）
 * @return 格式化后的时间字符串，根据时长自动选择合适的格式（如：1d2h3m4s 或 5m6s 等）
 */
inline std::string formatDuration(time_t durationSeconds) {
    durationSeconds = std::max(durationSeconds, 0LL); // 保证不出现负数
    auto parts      = decomposeDuration(durationSeconds);

    // 根据时间长度选择合适的格式
    if (parts.days > 0) return fmt::format("{}d{}h{}m{}s", parts.days, parts.hours, parts.minutes, parts.seconds);
    if (parts.hours > 0) return fmt::format("{}h{}m{}s", parts.hours, parts.minutes, parts.seconds);
    if (parts.minutes > 0) return fmt::format("{}m{}s", parts.minutes, parts.seconds);
    return fmt::format("{}s", parts.seconds);
}

/**
 * 格式化剩余时间为易读的字符串表示
 * @param totalSeconds 总时间（秒）
 * @return 剩余时间的格式化字符串
 */
inline std::string formatRemaining(time_t totalSeconds) {
    auto remaining = totalSeconds - nowSeconds(); // 计算剩余秒数
    return formatDuration(remaining);
}

/**
 * @brief 校验时间戳是否在合理范围内
 * @note 2000 ~ 2100 年之间
 */
inline bool ensureTimestamp(time_t ts) { return ts >= 946684800LL && ts <= 4102444800LL; }

/**
 * @brief 解析时间字符串 (支持 Unix 时间戳 或 YYYY-MM-DD HH:mm:ss)
 * @param time_str 输入字符串
 * @param isLocal 是否使用本地时间，默认为 true
 * @return system_clock::time_point 解析失败返回 epoch (0)
 */
inline std::chrono::system_clock::time_point parseTime(std::string_view time_str, bool isLocal = true) {
    using namespace std::chrono;

    if (time_str.empty()) return {};

    // 解析纯数字 Unix 时间戳 (Unix时间戳永远是绝对的/UTC，不需要受时区影响)
    time_t ts      = 0;
    auto [ptr, ec] = std::from_chars(time_str.data(), time_str.data() + time_str.size(), ts);

    // 如果 ptr 走到了结尾，说明整个字符串都是数字
    if (ec == std::errc{} && ptr == time_str.data() + time_str.size()) {
        if (ensureTimestamp(ts)) {
            return system_clock::from_time_t(ts);
        }
        return {}; // 时间戳不合理
    }

    // 尝试解析日期格式 YYYY-MM-DD HH:mm:ss 或 YYYY-MM-DD
    std::istringstream ss{std::string(time_str)};

    if (isLocal) {
#if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
        // 解析为无时区关联的本地时间
        local_seconds lt;
        ss >> std::chrono::parse("%Y-%m-%d %H:%M:%S", lt);
        if (ss.fail()) {
            ss.clear();
            ss.str(std::string(time_str));
            ss >> std::chrono::parse("%Y-%m-%d", lt);
        }

        if (!ss.fail()) {
            try {
                // 将本地时间通过当前时区转换回 UTC 系统时间点
                return current_zone()->to_sys(lt);
            } catch (const std::chrono::nonexistent_local_time&) {
                return {}; // 夏令时跳跃导致的“不存在时间”异常处理
            } catch (const std::chrono::ambiguous_local_time&) {
                return {}; // 夏令时回退导致的“重叠时间”异常处理
            }
        }
#else
        // 对于不支持时区库的编译器，使用 C 风格 API
        std::tm tm_buf{};
        ss >> std::get_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
        if (ss.fail()) {
            ss.clear();
            ss.str(std::string(time_str));
            ss >> std::get_time(&tm_buf, "%Y-%m-%d");
        }
        if (!ss.fail()) {
            tm_buf.tm_isdst = -1; // 让系统自动判断夏令时
            time_t t        = std::mktime(&tm_buf);
            if (t != -1) return system_clock::from_time_t(t);
        }
#endif
    } else {
        // 如果明确字符串是 UTC 时间，直接存入 sys_seconds
        sys_seconds st; // 等价于 time_point<system_clock, seconds>
        ss >> std::chrono::parse("%Y-%m-%d %H:%M:%S", st);
        if (ss.fail()) {
            ss.clear();
            ss.str(std::string(time_str));
            ss >> std::chrono::parse("%Y-%m-%d", st);
        }
        if (!ss.fail()) return st;
    }

    return {}; // 解析失败
}


} // namespace land::time_utils