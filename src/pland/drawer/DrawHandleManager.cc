#include "DrawHandleManager.h"

#include "DrawerType.h"
#include "detail/DebugShapeHandle.h"
#include "detail/DefaultParticleHandle.h"
#include "pland/PLand.h"
#include "pland/events/player/PlayerDeleteLandEvent.h"
#include "pland/land/Config.h"

#include "mc/world/actor/player/Player.h"

#include "ll/api/event/player/PlayerDisconnectEvent.h"
#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>


#include <memory>


namespace land {

struct DrawHandleManager::Impl {
    std::unordered_map<mce::UUID, std::unique_ptr<drawer::IDrawerHandle>> mDrawHandles;
    ll::event::ListenerPtr                                                mPlayerDeleteLandListener;
    ll::event::ListenerPtr                                                mPlayerDisconnectListener;

    std::unique_ptr<drawer::IDrawerHandle> _createHandle() const {
        auto backend = ConfigProvider::getDrawConfig().backend;
        switch (backend) {
        case DrawerType::DefaultParticle:
            return std::make_unique<drawer::detail::DefaultParticleHandle>();
        case DrawerType::DebugShape:
            return std::make_unique<drawer::detail::DebugShapeHandle>();
        }
        throw std::runtime_error("Unknown drawer type");
    }

    drawer::IDrawerHandle* getOrCreateHandle(Player& player) {
        auto iter = mDrawHandles.find(player.getUuid());
        if (iter == mDrawHandles.end()) {
            auto handle = _createHandle();
            handle->setTargetPlayer(player);
            iter = mDrawHandles.emplace(player.getUuid(), std::move(handle)).first;
        }
        return iter->second.get();
    }

    drawer::IDrawerHandle* tryGetHandle(mce::UUID const& uuid) const {
        auto iter = mDrawHandles.find(uuid);
        if (iter == mDrawHandles.end()) {
            return nullptr;
        }
        return iter->second.get();
    }

    bool removeHandle(mce::UUID const& uuid) { return mDrawHandles.erase(uuid) > 0; }

    void clear() { mDrawHandles.clear(); }


    void ensureDrawerBackendAvailable() const {
        auto& logger = PLand::getInstance().getSelf().getLogger();

        auto& conf = ConfigProvider::getDrawConfig();
        switch (conf.backend) {
        case DrawerType::DebugShape: {
            if (!drawer::detail::DebugShapeHandle::isDebugShapeLoaded()) {
                logger.warn(
                    "[DrawHandleManager] The DebugShape module is not loaded, and the plugin uses the built-in "
                    "particle "
                    "system!"
                );
                conf.backend = DrawerType::DefaultParticle;
                (void)ConfigProvider::save(PLand::getInstance().getSelf().getConfigDir());
            }
            break;
        }
        default:
            break;
        }
    }
};

DrawHandleManager::DrawHandleManager() : impl(std::make_unique<Impl>()) {
    impl->ensureDrawerBackendAvailable();
    impl->mPlayerDeleteLandListener =
        ll::event::EventBus::getInstance().emplaceListener<event::PlayerDeleteLandAfterEvent>(
            [this](event::PlayerDeleteLandAfterEvent& event) {
                if (auto h = impl->tryGetHandle(event.self().getUuid())) {
                    h->remove(event.land());
                }
            }
        );
    impl->mPlayerDisconnectListener =
        ll::event::EventBus::getInstance().emplaceListener<ll::event::PlayerDisconnectEvent>(
            [this](ll::event::PlayerDisconnectEvent& event) { impl->removeHandle(event.self().getUuid()); }
        );
}
DrawHandleManager::~DrawHandleManager() {
    ll::event::EventBus::getInstance().removeListener(impl->mPlayerDeleteLandListener);
    ll::event::EventBus::getInstance().removeListener(impl->mPlayerDisconnectListener);
}

drawer::IDrawerHandle* DrawHandleManager::getOrCreateHandle(Player& player) { return impl->getOrCreateHandle(player); }
drawer::IDrawerHandle* DrawHandleManager::tryGetHandle(mce::UUID const& uuid) { return impl->tryGetHandle(uuid); }

void DrawHandleManager::removeHandle(Player& player) { impl->removeHandle(player.getUuid()); }
void DrawHandleManager::removeHandle(mce::UUID const& uuid) { impl->removeHandle(uuid); }

void DrawHandleManager::removeAllHandle() { impl->clear(); }


} // namespace land
