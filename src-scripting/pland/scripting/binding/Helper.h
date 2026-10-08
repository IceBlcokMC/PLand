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

inline std::string format(jspp::Arguments const& args) {
    auto len = args.length();
    if (len == 0) [[unlikely]] {
        throw jspp::Exception{"log/format requires at least one argument", jspp::ExceptionType::TypeError};
    }

    auto f       = args[0];
    auto fmt_str = f.asString().getValue();

    fmt::dynamic_format_arg_store<fmt::format_context> store;
    for (size_t i = 1; i < len; ++i) {
        auto v = args[i];
        store.push_back(v.toString().getValue());
    }
    return fmt::vformat(fmt_str, store);
}

} // namespace land::scripting::helper