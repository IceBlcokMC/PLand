#pragma once
#include "pland/scripting/pch.h"

#include "fmt/format.h"
#include <fmt/args.h>

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

// fmt_start_offset is the index of the first argument to be formatted

// format("hello {0}", "world") -> "hello world"
template <size_t format_params_start_offset = 1>
inline std::string formatImpl(std::string_view fmt_str, jspp::Arguments const& args) {
    fmt::dynamic_format_arg_store<fmt::format_context> store;
    for (size_t i = format_params_start_offset; i < args.length(); ++i) {
        auto v = args[i];
        store.push_back(v.toString().getValue());
    }
    return fmt::vformat(fmt_str, store);
}

template <size_t fmt_start_offset = 0>
inline std::string format(jspp::Arguments const& args) {
    auto argc = args.length();
    if (argc <= fmt_start_offset) [[unlikely]] {
        throw jspp::Exception{"log/format requires at least one argument", jspp::ExceptionType::TypeError};
    }

    auto f       = args[fmt_start_offset];
    auto fmt_str = f.asString().getValue();

    constexpr auto skip_offset = fmt_start_offset + 1;
    return formatImpl<skip_offset>(fmt_str, args);
}

} // namespace land::scripting::helper