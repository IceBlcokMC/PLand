#pragma once
#include <type_traits>

namespace land ::reflect {

template <typename T>
struct member_pointer_traits;

template <typename C, typename F>
struct member_pointer_traits<F C::*> {
    using class_type = C;
    using field_type = F;
};

template <typename T>
using member_pointer_class_t = typename member_pointer_traits<std::remove_cvref_t<T>>::class_type;

template <typename T>
using member_pointer_field_t = typename member_pointer_traits<std::remove_cvref_t<T>>::field_type;


} // namespace land::reflect


namespace {
struct Foo {
    int        a;
    const bool b;
};
static_assert(std::is_same_v<land::reflect::member_pointer_class_t<decltype(&Foo::a)>, Foo>);
static_assert(std::is_same_v<land::reflect::member_pointer_field_t<decltype(&Foo::a)>, int>);
static_assert(std::is_same_v<land::reflect::member_pointer_field_t<decltype(&Foo::b)>, const bool>);
} // namespace