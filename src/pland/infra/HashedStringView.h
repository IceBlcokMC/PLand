#pragma once
#include <cstdint>
#include <string_view>

#include <mc/deps/core/string/HashedString.h>


namespace land {


class HashedStringView {
public:
    std::string_view const mStr;
    uint64_t const         mHash;

    constexpr HashedStringView(std::string_view str) noexcept : mStr(str), mHash(HashedString::computeHash(str)) {}

    constexpr HashedStringView(std::string_view str, uint64_t hash) noexcept : mStr(str), mHash(hash) {}

    constexpr bool operator==(const HashedStringView& other) const { return other.mHash == mHash; }
    constexpr bool operator==(const ::HashedString& other) const { return other.mStrHash == mHash; }
};


} // namespace land


namespace std {

template <>
struct hash<land::HashedStringView> {
    constexpr size_t operator()(land::HashedStringView const& hs) const noexcept { return hs.mHash; }
};

} // namespace std