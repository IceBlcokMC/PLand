#pragma once
#include <memory>

#include "Global.h"
#include "ll/api/mod/NativeMod.h"


namespace land::service {
class ServiceLocator;
}

namespace ll::thread {
class ThreadPoolExecutor;
}

#ifdef PLAND_SCRIPTING
namespace land::scripting {
class Bootstrap;
}
#endif


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

#ifdef PLAND_SCRIPTING
    scripting::Bootstrap& getScriptingBootstrap();
    void reloadScripting();
#endif


private:
    struct Impl;
    std::unique_ptr<Impl> mImpl{nullptr};
};

} // namespace land
