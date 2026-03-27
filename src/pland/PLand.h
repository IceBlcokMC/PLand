#pragma once
#include <memory>

#include "Global.h"
#include "ll/api/mod/NativeMod.h"


namespace land {
namespace service {
class ServiceLocator;
}
} // namespace land

namespace ll {
namespace thread {
class ThreadPoolExecutor;
}
} // namespace ll


namespace land {

namespace internal {
class SafeTeleport;
}


class PLand {
    PLand();

public: /* private */
    [[nodiscard]] ll::mod::NativeMod& getSelf() const;

    bool load();
    bool enable();
    bool disable();
    bool unload();

public: /* public */
    LDAPI static PLand& getInstance();

    LDNDAPI class SelectorManager*   getSelectorManager() const;
    LDNDAPI class LandRegistry&      getLandRegistry() const;
    LDNDAPI class DrawHandleManager* getDrawHandleManager() const;

    LDNDAPI ll::thread::ThreadPoolExecutor& getThreadPool() const;

    LDNDAPI service::ServiceLocator& getServiceLocator() const;

    LDNDAPI internal::SafeTeleport& getSafeTeleport() const;

    LDAPI bool loadConfig();
    LDAPI bool saveConfig();

#ifdef LD_DEVTOOL
    void setDevToolVisible(bool visible);
#endif

private:
    struct Impl;
    std::unique_ptr<Impl> mImpl{nullptr};
};

} // namespace land
