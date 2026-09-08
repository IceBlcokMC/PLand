#pragma once
#include "IHookGuard.h"
#include "pland/Global.h"

#include "InterceptorConfig.h"

#include <ll/api/event/ListenerBase.h>
#include <memory>
#include <type_traits>

namespace land::internal::interceptor {

class EventInterceptor final {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    LD_DISABLE_COPY_AND_MOVE(EventInterceptor);
    EventInterceptor();
    ~EventInterceptor();

    template <bool InterceptorConfig::Listeners::* E, typename Factory>
        requires std::invocable<Factory>
    void registerListenerIf(Factory&& factory) {
        auto enabled    = InterceptorConfig::cfg.listeners.*E;
        auto registered = isListenerAlreadyRegistered(E);
        if (enabled && !registered) {
            _registerListener(E, std::forward<Factory>(factory)());
        } else if (!enabled && registered) {
            _unregisterListener(E);
        }
    }

    template <bool InterceptorConfig::Hooks::* E, Hookable... Ts>
        requires(sizeof...(Ts) >= 1)
    void registerHookIf(IHookGuard::Cleanup finalizer = nullptr) {
        auto resolve = [&]() -> std::unique_ptr<IHookGuard> {
            if constexpr (sizeof...(Ts) == 1) {
                return std::make_unique<HookGuardImpl<Ts...>>(finalizer);
            } else {
                return std::make_unique<MultiHookGuardImpl<Ts...>>(finalizer);
            }
        };

        auto enabled    = InterceptorConfig::cfg.hooks.*E;
        auto registered = isHookAlreadyRegistered(E);
        if (enabled && !registered) {
            _registerHook(E, std::move(resolve()));
        } else if (!enabled && registered) {
            _unregisterHook(E);
        }
    }

    void reload();

private:
    bool isListenerAlreadyRegistered(bool InterceptorConfig::Listeners::* configure) const;
    bool isHookAlreadyRegistered(bool InterceptorConfig::Hooks::* configure) const;

    void _registerListener(bool InterceptorConfig::Listeners::* configure, ll::event::ListenerPtr listener);
    void _registerHook(bool InterceptorConfig::Hooks::* configure, std::unique_ptr<IHookGuard> hookGuard);

    void _unregisterListener(bool InterceptorConfig::Listeners::* configure);
    void _unregisterHook(bool InterceptorConfig::Hooks::* configure);

    void setupLLPlayerListeners();
    void setupLLEntityListeners();
    void setupLLWorldListeners();

    void setupIlaPlayerListeners();
    void setupIlaEntityListeners();
    void setupIlaWorldListeners();

    void setupHooks();
};

} // namespace land::internal::interceptor
