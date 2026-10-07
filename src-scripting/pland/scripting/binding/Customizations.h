#pragma once

// 这个头由 pch 引入, 所以自己带齐依赖, 不回头包含 pch
#include "jspp/binding/NativeInstanceImpl.h"

#include "mc/deps/ecs/WeakEntityRef.h"
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

    explicit PlayerInstanceImpl(jspp::ClassMeta const* meta, Player& player)
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
struct NativeInstanceFactory<Player> {
    template <typename U>
    static std::unique_ptr<NativeInstance>
    create(U&& value, ReturnValuePolicy /*policy*/, detail::ResolvedCastSource const& resolved) {
        using Raw = std::remove_cvref_t<U>;
        static_assert(
            std::is_pointer_v<Raw> || std::is_lvalue_reference_v<Raw>,
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
