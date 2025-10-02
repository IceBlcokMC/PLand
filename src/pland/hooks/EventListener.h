
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
class HookGuard {
    std::function<void()> mUnhookFunc;

public:
    explicit HookGuard(std::function<void()> registerFunc, std::function<void()> unregisterFunc)
        : mUnhookFunc(std::move(unregisterFunc)) {
        registerFunc();
    }

    ~HookGuard() {
        if (mUnhookFunc) {
            mUnhookFunc();
        }
    }

    // 禁止拷贝和赋值
    HookGuard(const HookGuard&) = delete;
    HookGuard& operator=(const HookGuard&) = delete;

    // 允许移动
    HookGuard(HookGuard&& other) noexcept : mUnhookFunc(std::move(other.mUnhookFunc)) {
        other.mUnhookFunc = nullptr;
    }
    HookGuard& operator=(HookGuard&& other) noexcept {
        if (this != &other) {
            if (mUnhookFunc) {
                mUnhookFunc();
            }
            mUnhookFunc       = std::move(other.mUnhookFunc);
            other.mUnhookFunc = nullptr;
        }
        return *this;
    }
};


/**
 * @brief EventListener
 * 领地事件监听器集合，RAII 管理事件监听器的注册和注销。
 */
class EventListener {
    std::vector<ll::event::ListenerPtr> mListenerPtrs;
    std::vector<HookGuard>              mHookGuards; // 新增用于管理 hook 的 RAII 对象

    void RegisterListenerIf(bool need, std::function<ll::event::ListenerPtr()> const& factory);
    void RegisterHookIf(bool need, std::function<void()> registerFunc, std::function<void()> unregisterFunc);


    // 为不同事件类别声明注册函数
    // 按照 ll 和 ila 库进行拆分
    void registerLLSessionListeners();

    void registerLLPlayerListeners();
    void registerILAPlayerListeners();

    void registerLLEntityListeners();
    void registerILAEntityListeners();

    void registerLLWorldListeners();
    void registerILAWorldListeners();


public:
    LD_DISABLE_COPY(EventListener);

    LDAPI explicit EventListener();
    LDAPI ~EventListener();

    void registerHooks();
    void unregisterHooks();
};


} // namespace land
