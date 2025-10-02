
#include "pland/hooks/EventListener.h"
#include "ll/api/event/EventBus.h"
#include "pland/infra/Config.h"
#include "pland/hooks/hook.h"
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
}

EventListener::~EventListener() {
    auto& bus = ll::event::EventBus::getInstance();
    for (auto& ptr : mListenerPtrs) {
        bus.removeListener(ptr);
    }
}

void EventListener::registerHooks() {
    if (Config::cfg.hooks.registerMobHurtHook) {
        registerMobHurtHook();
    }
    if (Config::cfg.hooks.registerFishingHookHitHook) {
        registerOnFishingHookHitHook();
    }
}

void EventListener::unregisterHooks() {
    if (Config::cfg.hooks.registerMobHurtHook) {
        unregisterMobHurtHook();
    }
    if (Config::cfg.hooks.registerFishingHookHitHook) {
        unregisterOnFishingHookHitHook();
    }
}

void EventListener::RegisterListenerIf(bool need, std::function<ll::event::ListenerPtr()> const& factory) {
    if (need) {
        auto listenerPtr = factory();
        mListenerPtrs.push_back(std::move(listenerPtr));
    }
}

} // namespace land
