#pragma once
#include "pland/Global.h"

#include <concepts>
#include <type_traits>

namespace land::internal::interceptor {


struct IHookGuard {
    using Cleanup = void (*)();

    explicit IHookGuard() = delete;

    virtual ~IHookGuard() {
        if (finalizer_) finalizer_();
    }

protected:
    Cleanup finalizer_{nullptr};

    explicit IHookGuard(Cleanup finalizer) : finalizer_(finalizer) {}
};

template <typename T>
concept Hookable = requires(T) {
    { T::hook() };
    { T::unhook() };
};

template <Hookable T>
struct HookGuardImpl final : IHookGuard {
    HookGuardImpl(Cleanup finalizer = nullptr) : IHookGuard(finalizer) { T::hook(); }
    ~HookGuardImpl() override { T::unhook(); }
};

template <Hookable... Ts>
struct MultiHookGuardImpl final : IHookGuard {
    MultiHookGuardImpl(Cleanup finalizer = nullptr) : IHookGuard(finalizer) { (Ts::hook(), ...); }
    ~MultiHookGuardImpl() override { (Ts::unhook(), ...); }
};


} // namespace land::internal::interceptor