
#pragma once
#include "ll/api/event/ListenerBase.h"
#include "pland/Global.h"
#include <functional>
#include <vector>


namespace land {

/**
 * @brief HookGuard
 * RAII 管理 hook 的注册和注销。
 */
class HookGuard final {
public:
    using HookFunc = void (*)();

    explicit inline HookGuard(HookFunc hook, HookFunc unhook) : mUnhook(unhook) {
        if (hook) hook();
    }

    ~HookGuard() {
        if (mUnhook) mUnhook();
    }

    HookGuard(const HookGuard&)            = delete;
    HookGuard& operator=(const HookGuard&) = delete;

    HookGuard(HookGuard&& other) noexcept : mUnhook(other.mUnhook) { other.mUnhook = nullptr; }
    HookGuard& operator=(HookGuard&& other) noexcept {
        if (this != &other) {
            if (mUnhook) {
                mUnhook();
            }
            mUnhook       = other.mUnhook;
            other.mUnhook = nullptr;
        }
        return *this;
    }

private:
    HookFunc mUnhook;
};

template <typename T>
concept Hookable = requires(T) {
    { T::hook() };
    { T::unhook() };
};

/**
 * @brief EventListener
 * 领地事件监听器集合，RAII 管理事件监听器的注册和注销。
 */
class EventListener {
    std::vector<ll::event::ListenerPtr> mListenerPtrs;
    std::vector<HookGuard>              mHookGuards;

    void RegisterListenerIf(bool need, std::function<ll::event::ListenerPtr()> const& factory);
    void RegisterHookIf(bool need, HookGuard::HookFunc setup, HookGuard::HookFunc teardown);

    template <Hookable T>
    void RegisterHookIf(bool need) {
        this->RegisterHookIf(need, []() { T::hook(); }, []() { T::unhook(); });
    }


    // 为不同事件类别声明注册函数
    // 按照 ll 和 ila 库进行拆分
    void registerLLSessionListeners();

    void registerLLPlayerListeners();
    void registerILAPlayerListeners();

    void registerLLEntityListeners();
    void registerILAEntityListeners();

    void registerLLWorldListeners();
    void registerILAWorldListeners();

    void registerHooks(); // impl in Hook.cc

public:
    LD_DISABLE_COPY(EventListener);

    LDAPI explicit EventListener();
    LDAPI ~EventListener();
};


} // namespace land
