#pragma once
#include <memory>

#include "Global.h"
#include "ll/api/mod/NativeMod.h"

#ifdef LD_DEVTOOL
namespace devtool {
class DevToolApp;
}
#endif

namespace land {

class LandRegistry;
class EventListener;
class LandScheduler;
class SafeTeleport;
class SelectorManager;
class DrawHandleManager;

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

    LDAPI void onConfigReload();

    LDNDAPI SafeTeleport*      getSafeTeleport() const;
    LDNDAPI LandScheduler*     getLandScheduler() const;
    LDNDAPI SelectorManager*   getSelectorManager() const;
    LDNDAPI LandRegistry*      getLandRegistry() const;
    LDNDAPI DrawHandleManager* getDrawHandleManager() const;

#ifdef LD_DEVTOOL
    [[nodiscard]] devtool::DevToolApp* getDevToolApp() const;
#endif

private:
    ll::mod::NativeMod& mSelf;

    std::unique_ptr<LandRegistry>      mLandRegistry{nullptr};
    std::unique_ptr<EventListener>     mEventListener{nullptr};
    std::unique_ptr<LandScheduler>     mLandScheduler{nullptr};
    std::unique_ptr<SafeTeleport>      mSafeTeleport{nullptr};
    std::unique_ptr<SelectorManager>   mSelectorManager{nullptr};
    std::unique_ptr<DrawHandleManager> mDrawHandleManager{nullptr};

#ifdef LD_DEVTOOL
    std::unique_ptr<devtool::DevToolApp> mDevToolApp{nullptr};
#endif
};

} // namespace land
