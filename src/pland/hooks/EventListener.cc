
#include "pland/hooks/EventListener.h"
#include "ll/api/event/EventBus.h"
#include "pland/hooks/hook.h"
#include "pland/infra/Config.h"
#include <functional>


namespace land {

EventListener::EventListener() {
    // 调用所有分类的注册函数
    registerLLSessionListeners();

    registerLLPlayerListeners();
    registerILAPlayerListeners();

    registerLLEntityListeners();
    registerILAEntityListeners();

    registerLLWorldListeners();
    registerILAWorldListeners();

    // 注册所有需要的 hook
    registerHooks();
}

EventListener::~EventListener() {
}

void EventListener::registerHooks() {
    RegisterHookIf(
        Config::cfg.hooks.registerMobHurtHook, [] { registerMobHurtHook(); }, [] { unregisterMobHurtHook(); }
    );
    RegisterHookIf(
        Config::cfg.hooks.registerFishingHookHitHook,
        [] { registerOnFishingHookHitHook(); },
        [] { unregisterOnFishingHookHitHook(); }
    );
    RegisterHookIf(
        Config::cfg.hooks.registerLayEggGoalHook, [] { registeronLayEggGoalHook(); }, [] { unregisteronLayEggGoalHook(); }
    );
}

void EventListener::unregisterHooks() {
}

void EventListener::RegisterListenerIf(bool need, std::function<ll::event::ListenerPtr()> const& factory) {
    if (need) {
        auto listenerPtr = factory();
        mListenerPtrs.push_back(std::move(listenerPtr));
    }
}

void EventListener::RegisterHookIf(
    bool need, std::function<void()> registerFunc, std::function<void()> unregisterFunc
) {
    if (need) {
        mHookGuards.emplace_back(std::move(registerFunc), std::move(unregisterFunc));
    }
}

} // namespace land
