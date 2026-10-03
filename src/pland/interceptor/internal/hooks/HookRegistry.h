#pragma once
#include <type_traits>
#include <vector>

namespace land::internal::interceptor {
class EventInterceptor;
}

namespace land::internal::interceptor::hooks {

using HookRegisterFn = std::add_pointer_t<void(EventInterceptor& interceptor)>;

class HookRegistry {
public:
    [[nodiscard]] inline static auto& getEntries() {
        static std::vector<HookRegisterFn> entries;
        return entries;
    }

    inline static void addHook(HookRegisterFn fn) { getEntries().push_back(fn); }

    inline static void dispatchAllHooks(EventInterceptor& interceptor) {
        for (auto fn : getEntries()) {
            if (fn) fn(interceptor);
        }
    }
};

struct AutoHookRegister {
    inline explicit AutoHookRegister(HookRegisterFn fn) { HookRegistry::addHook(fn); }
};

} // namespace land::internal::interceptor::hooks

#define LAND_CONCAT_IMPL(x, y) x##y
#define LAND_CONCAT(x, y)      LAND_CONCAT_IMPL(x, y)

#define LAND_REGISTER_HOOK(...)                                                                                        \
    static const ::land::internal::interceptor::hooks::AutoHookRegister LAND_CONCAT(_auto_hook_reg_, __COUNTER__)(     \
        __VA_ARGS__                                                                                                    \
    )
