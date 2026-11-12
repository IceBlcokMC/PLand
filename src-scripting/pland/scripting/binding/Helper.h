#pragma once
#include "pland/scripting/pch.h"

#include "fmt/format.h"
#include <fmt/args.h>

#include <ll/api/Expected.h>

namespace land::scripting::helper {

/**
 * auto generate enum meta
 * @tparam T enum
 * @param ename script enum name
 * @return EnumMeta
 * @warning Only support enum range [-128, 128]
 */
template <typename T>
    requires std::is_enum_v<T>
jspp::EnumMeta auto_gen_enum_def(std::string_view ename) {
    jspp::binding::EnumMetaBuilder def = jspp::binding::defEnum<T>(ename);

    constexpr auto entries = magic_enum::enum_entries<T>();
    for (auto [value, name] : entries) {
        def.value(std::string{name}, value);
    }
    return def.build();
}

inline std::string toPOSIXPath(std::filesystem::path const& p) {
    auto        u8 = p.generic_u8string();
    std::string s(reinterpret_cast<char const*>(u8.data()), u8.size());
    assert(s.find('\\') == std::string::npos); // 防呆：反斜杠不允许外泄到脚本
    return s;
}

/**
 * 格式化字符串函数实现
 * @tparam format_params_start_offset 参数起始位置
 * @tparam named_format 是否支持基于名称的格式化
 *         如果启用 named_format，则格式化字符串中需要使用 {name} 的形式，例如: "hello {name}"
 *         否则，格式化字符串中需要使用 {0} 的形式，例如: "hello {0}"
 * @param fmt_str 待格式化的字符串
 * @param args 脚本参数
 */
template <size_t format_params_start_offset = 1, bool named_format = false>
inline std::string formatImpl(std::string_view fmt_str, jspp::Arguments const& args) {
    fmt::dynamic_format_arg_store<fmt::format_context> store;

    auto const argc = args.length();
    // 参数起始位置之前的参数属于调用方必须提供的前置参数
    if (argc < format_params_start_offset) [[unlikely]] {
        throw jspp::Exception{"format arguments are incomplete", jspp::ExceptionType::TypeError};
    }
    // 没有格式化参数，直接格式化
    if (argc == format_params_start_offset) {
        return fmt::vformat(fmt_str, store);
    }

    auto const param_c = argc - format_params_start_offset;

    if constexpr (named_format) {
        // format("hello {name}", {name: "world"})
        // named_format 模式只接受一个参数对象
        if (param_c != 1) [[unlikely]] {
            throw jspp::Exception{"format requires exactly one named arguments object", jspp::ExceptionType::TypeError};
        }

        auto maybe_object = args[format_params_start_offset];
        if (!maybe_object.isObject()) [[unlikely]] {
            throw jspp::Exception{"format requires named arguments to be an object", jspp::ExceptionType::TypeError};
        }
        auto named_object = maybe_object.asObject();
        auto names        = named_object.getOwnPropertyNames();

        store.reserve(names.size(), names.size());
        for (auto const& key : names) {
            auto name  = key.getValue();
            auto value = named_object.get(key).toString().getValue();
            store.push_back(fmt::arg(name.c_str(), value));
        }
    } else {
        // format("hello {0}", "world")
        store.reserve(param_c, 0);
        for (size_t i = format_params_start_offset; i < args.length(); ++i) {
            auto value = args[i].toString().getValue();
            store.push_back(value);
        }
    }
    return fmt::vformat(fmt_str, store);
}

/**
 * 格式化字符串
 * @note 此接口仅支持基于下标的格式化，不支持基于名称的格式化
 * @tparam fmt_start_offset 格式化字符串起始位置
 *         用于跳过一些前置参数，例如: format("zh_CN", "hello {0}", "world") -> fmt_start_offset=1
 * @param args 脚本回调参数
 */
template <size_t fmt_start_offset = 0>
inline std::string format(jspp::Arguments const& args) {
    auto argc = args.length() - fmt_start_offset;
    if (argc < 1) {
        throw jspp::Exception{"Formatting requires a string to be formatted", jspp::ExceptionType::TypeError};
    }

    auto maybe_fstr = args[fmt_start_offset];
    if (!maybe_fstr.isString()) {
        throw jspp::Exception{"The string to be formatted must be a string", jspp::ExceptionType::TypeError};
    }
    auto fmt_str = maybe_fstr.asString().getValue();

    constexpr auto skip_offset = fmt_start_offset + 1;
    return formatImpl<skip_offset>(fmt_str, args);
}


// unsafe API

inline constexpr auto kStructuredCloneFlags = JS_WRITE_OBJ_REFERENCE;

/**
 * 将一个值结构化克隆到缓冲区
 * @param engine 待克隆的值所属的引擎
 * @param val 待克隆的值(需要与 engine 匹配)
 * @param buf_len 缓冲区长度
 * @return 二进制缓冲区指针
 *
 * @warning 返回的 buffer 需要由对应的引擎(context)释放
 */
[[nodiscard]] inline uint8_t*
unsafe_StructuredCloneToBuffer(jspp::Engine& engine, jspp::Local<jspp::Value> const& val, size_t& buf_len) noexcept {
    return JS_WriteObject(
        engine.context(),
        &buf_len,
        jspp::qjs_backend::QjsHelper::peekValue(val),
        kStructuredCloneFlags
    );
}

/**
 * 释放一个结构化克隆的缓冲区
 * @param engine 待释放的缓冲区所属的引擎
 * @param buf 缓冲区指针
 */
inline void unsafe_StructuredCloneFreeBuffer(jspp::Engine& engine, uint8_t* buf) noexcept {
    if (buf == nullptr) {
        return;
    }
    // jspp::EngineScope lock{engine};
    js_free(engine.context(), buf);
}

/**
 * 将一个结构化克隆的缓冲区反序列化为一个值
 * @param source 缓冲区所属的引擎
 * @param buf 缓冲区指针
 * @param len 缓冲区长度
 * @param target 目标引擎
 * @param targetVal 目标值
 * @param freeBufferInplace 是否在反序列化后立即释放缓冲区
 *
 * @throw jspp::Exception 如果反序列化失败则抛出异常
 * @note 此函数要求 EngineScope 位于目标引擎(target)上下文中，否则异常传递可能跨越引擎上下文
 */
inline void unsafe_StructuredCloneBufferToValue(
    jspp::Engine&             source,
    uint8_t*                  buf,
    size_t                    len,
    jspp::Engine&             target,
    jspp::Local<jspp::Value>& targetVal,
    bool                      freeBufferInplace
) {
    if (jspp::EngineScope::currentEngine() != &target) [[unlikely]] {
        throw std::logic_error{"current engine scope is not target engine scope"};
    }

    auto rawVal = JS_ReadObject(target.context(), buf, len, kStructuredCloneFlags);
    if (freeBufferInplace) {
        unsafe_StructuredCloneFreeBuffer(source, buf);
    }

    jspp::qjs_backend::QjsHelper::rethrowException(rawVal); // throw exception if failed
    targetVal = jspp::ValueHelper::wrap<jspp::Value>(rawVal);
}

/**
 * 将一个值结构化克隆到另一个引擎(跨引擎上下文)
 * @param source 源引擎
 * @param sourceVal 源值
 * @param target 目标引擎
 * @param targetVal 目标值
 *
 * @throw jspp::Exception 如果反序列化失败则抛出异常
 * @note 此函数要求 EngineScope 位于目标引擎(target)上下文中，否则异常传递可能跨越引擎上下文
 * @note 注意 Local<T> 句柄的析构函数和引擎作用域绑定，请在匹配的引擎上下文中使用/释放 Local<T> 句柄
 */
inline void unsafe_StructuredCloneValue(
    jspp::Engine&                   source,
    jspp::Local<jspp::Value> const& sourceVal,
    jspp::Engine&                   target,
    jspp::Local<jspp::Value>&       targetVal
) {
    size_t len = 0;
    auto   buf = unsafe_StructuredCloneToBuffer(source, sourceVal, len);
    if (!buf) [[unlikely]] {
        throw jspp::Exception("Failed to serialize value");
    }
    unsafe_StructuredCloneBufferToValue(source, buf, len, target, targetVal, true);
}

/**
 * 将多个值结构化克隆到另一个引擎(跨引擎上下文)
 * @param s_args 源参数列表
 * @param target 目标引擎
 * @param t_out 输出参数列表
 *
 * @throw jspp::Exception 如果反序列化失败则抛出异常
 * @note 此函数要求 EngineScope 位于目标引擎(target)上下文中，否则异常传递可能跨越引擎上下文
 * @note 注意 Local<T> 句柄的析构函数和引擎作用域绑定，请在匹配的引擎上下文中使用/释放 Local<T> 句柄
 */
inline void unsafe_StructureCloneValues(
    jspp::Arguments const&                 s_args,
    jspp::Engine&                          target,
    std::vector<jspp::Local<jspp::Value>>& t_out
) {
    auto s_len = s_args.length();
    if (s_len == 0) {
        return;
    }

    t_out.resize(s_len);

    auto& s_engine = *s_args.engine();
    for (size_t i = 0; i < s_len; ++i) {
        auto s_value = s_args[i];
        unsafe_StructuredCloneValue(s_engine, s_value, target, t_out[i]);
    }
}

} // namespace land::scripting::helper