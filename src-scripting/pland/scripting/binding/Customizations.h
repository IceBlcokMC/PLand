#pragma once

// 这个头由 pch 引入, 所以自己带齐依赖, 不回头包含 pch
#include "jspp/binding/NativeInstanceImpl.h"

#include "mc/deps/ecs/WeakEntityRef.h"
#include <mc/server/commands/CommandOrigin.h>
#include <mc/world/actor/player/Player.h>

#include <memory>
#include <typeindex>
#include <typeinfo>
#include <utility>

namespace land::scripting {

class PlayerInstanceImpl final : public jspp::NativeInstance {
    WeakRef<EntityContext> mWeakEntity{};

    auto resolve() const { return mWeakEntity.tryUnwrap<Player>(); }

public:
    PlayerInstanceImpl(jspp::ClassMeta const* meta, WeakRef<EntityContext> weak)
    : jspp::NativeInstance(meta),
      mWeakEntity(std::move(weak)) {}

    explicit PlayerInstanceImpl(jspp::ClassMeta const* meta, Player const& player)
    : PlayerInstanceImpl(meta, player.getEntityContext().getWeakRef()) {}

    void  invalidate() override { mWeakEntity = {}; }
    bool  is_expired() const override { return !resolve().has_value(); }
    bool  is_owned() const override { return false; }
    bool  is_const() const override { return false; }
    void* release_ownership() override { return nullptr; }

    std::type_index type_id() const override { return typeid(Player); }

    void* cast(std::type_index target) const override {
        auto ptr = resolve();
        if (!ptr) {
            return nullptr;
        }
        if (target == typeid(Player)) {
            return ptr.as_ptr();
        }
        return meta_ ? meta_->castTo(ptr.as_ptr(), target) : nullptr;
    }

    std::unique_ptr<jspp::NativeInstance> clone() const override {
        return std::make_unique<PlayerInstanceImpl>(meta_, mWeakEntity);
    }
};

} // namespace land::scripting

namespace jspp::binding::traits {

/// BDS 类型的 RTTI 不可用: 不做 dynamic_cast, 指针原样返回, 类型按静态类型判定
template <>
struct PolymorphicTypeHook<Player> {
    static void const* get(Player const* src, std::type_info const*& type) {
        type = nullptr;
        return src;
    }
};
template <>
struct PolymorphicTypeHook<CommandOrigin> {
    static void const* get(CommandOrigin const* src, std::type_info const*& type) {
        type = nullptr;
        return src;
    }
};

template <>
struct NativeInstanceFactory<Player> {
    template <typename U>
    static std::unique_ptr<NativeInstance>
    create(U&& value, ReturnValuePolicy /*policy*/, detail::ResolvedCastSource const& resolved) {
        using Raw = std::remove_cvref_t<U>;
        static_assert(
            std::is_pointer_v<Raw> || std::is_lvalue_reference_v<U>,
            "Player can only be exposed to JS as a pointer or an lvalue reference"
        );

        if constexpr (std::is_pointer_v<Raw>) {
            if (!value) return nullptr;
            return std::make_unique<land::scripting::PlayerInstanceImpl>(resolved.meta, *value);
        } else {
            return std::make_unique<land::scripting::PlayerInstanceImpl>(resolved.meta, value);
        }
    }
};

} // namespace jspp::binding::traits

namespace land::scripting {
template <typename T, auto Member>
using VectorMemberType_t = std::remove_cvref_t<decltype(std::declval<T>().*Member)>;

template <typename T>
concept Vector2Like = requires(T v) {
    { v.x };
    { v.z };
};

template <typename T>
concept Vector3Like = Vector2Like<T> && requires(T v) {
    { v.y };
};
} // namespace land::scripting

namespace jspp::binding {

template <land::scripting::Vector2Like T>
struct TypeConverter<T> {
    static Local<Value> toJs(T const& v, ReturnValuePolicy /* policy */, Local<Value> const& /* parent */) {
        auto obj = Object::newObject();
        obj.set(String::newString("x"), Number::newNumber(v.x));
        obj.set(String::newString("z"), Number::newNumber(v.z));
        return obj;
    }
    static T toCpp(Local<Value> const& v) {
        auto obj = v.asObject();
        return {
            static_cast<land::scripting::VectorMemberType_t<T, &T::x>>(
                obj.get(String::newString("x")).asNumber().getValueAs<land::scripting::VectorMemberType_t<T, &T::x>>()
            ),
            static_cast<land::scripting::VectorMemberType_t<T, &T::z>>(
                obj.get(String::newString("z")).asNumber().getValueAs<land::scripting::VectorMemberType_t<T, &T::z>>()
            ),
        };
    }
};
template <land::scripting::Vector3Like T>
struct TypeConverter<T> {
    static Local<Value> toJs(T const& v, ReturnValuePolicy /* policy */, Local<Value> const& /* parent */) {
        auto obj = Object::newObject();
        obj.set(String::newString("x"), Number::newNumber(v.x));
        obj.set(String::newString("y"), Number::newNumber(v.y));
        obj.set(String::newString("z"), Number::newNumber(v.z));
        return obj;
    }
    static T toCpp(Local<Value> const& v) {
        auto obj = v.asObject();
        return {
            static_cast<land::scripting::VectorMemberType_t<T, &T::x>>(
                obj.get(String::newString("x")).asNumber().getValueAs<land::scripting::VectorMemberType_t<T, &T::x>>()
            ),
            static_cast<land::scripting::VectorMemberType_t<T, &T::y>>(
                obj.get(String::newString("y")).asNumber().getValueAs<land::scripting::VectorMemberType_t<T, &T::y>>()
            ),
            static_cast<land::scripting::VectorMemberType_t<T, &T::z>>(
                obj.get(String::newString("z")).asNumber().getValueAs<land::scripting::VectorMemberType_t<T, &T::z>>()
            ),
        };
    }
};
template <>
struct TypeConverter<DimensionType> {
    static Local<Value> toJs(DimensionType const& v, ReturnValuePolicy /* policy */, Local<Value> const& /* parent */) {
        return Number::newNumber(static_cast<int>(v));
    }
    static DimensionType toCpp(Local<Value> const& v) { return static_cast<DimensionType>(v.asNumber().getInt32()); }
};
template <>
struct TypeConverter<mce::UUID> {
    static Local<Value> toJs(mce::UUID const& uuid, ReturnValuePolicy /* policy */, Local<Value> const& /* parent */) {
        return jspp::String::newString(uuid.asString());
    }
    static mce::UUID toCpp(Local<Value> const& value) {
        auto str = value.asString().getValue();
        return mce::UUID::fromString(str);
    }
};

} // namespace jspp::binding