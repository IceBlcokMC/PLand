#include "SelectorManager.h"
#include "ll/api/event/ListenerBase.h"
#include "pland/PLand.h"
#include "pland/infra/Debouncer.h"
#include "pland/land/Config.h"
#include "pland/utils/McUtils.h"


#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/coro/InterruptableSleep.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerDisconnectEvent.h"
#include "ll/api/event/player/PlayerInteractBlockEvent.h"
#include "ll/api/thread/ServerThreadExecutor.h"


#include "mc/world/actor/player/Player.h"

#include <atomic>


namespace land {

struct SelectorManager::Impl {
    std::unordered_map<mce::UUID, std::unique_ptr<ISelector>> mSelectors{};
    std::unordered_map<mce::UUID, Debouncer>                  mStabilization{};
    ll::event::ListenerPtr                                    mListener{nullptr};
    std::shared_ptr<std::atomic<bool>>                        mCoroStop{nullptr};
    std::shared_ptr<ll::coro::InterruptableSleep>             mInterruptableSleep{nullptr};

    ll::event::ListenerPtr mPlayerDisconnectListener;
};

SelectorManager::SelectorManager() : impl(std::make_unique<Impl>()) {
    impl->mListener = ll::event::EventBus::getInstance().emplaceListener<ll::event::PlayerInteractBlockEvent>(
        [this](ll::event::PlayerInteractBlockEvent const& ev) {
            auto& player = ev.self();

            if (player.isSimulatedPlayer() || !hasSelector(player)) {
                return;
            }

            // 防抖
            {
                auto iter = impl->mStabilization.find(player.getUuid());
                if (iter == impl->mStabilization.end()) {
                    iter = impl->mStabilization.emplace(player.getUuid(), 250).first; // ms
                }
                if (!iter->second.ready()) {
                    return;
                }
            }

            auto& itemTypeName = ConfigProvider::getSelectionConfig().item;
            if (ev.item().getTypeName() != itemTypeName) {
                return;
            }

            auto selector = getSelector(player);
            if (!selector) {
                return;
            }

            if (selector->isPointABSet()) {
                mc_utils::executeCommand("pland buy", player); // TODO: 优化
                return;
            }

            if (!selector->isPointASet()) {
                selector->setPointA(ev.blockPos());
            } else if (!selector->isPointBSet()) {
                selector->setPointB(ev.blockPos());
            }
        }
    );

    impl->mPlayerDisconnectListener =
        ll::event::EventBus::getInstance().emplaceListener<ll::event::PlayerDisconnectEvent>(
            [this](ll::event::PlayerDisconnectEvent const& ev) { this->stopSelection(ev.self()); }
        );

    impl->mCoroStop           = std::make_shared<std::atomic<bool>>(false);
    impl->mInterruptableSleep = std::make_shared<ll::coro::InterruptableSleep>();
    ll::coro::keepThis([sleep = impl->mInterruptableSleep, stop = impl->mCoroStop, this]() -> ll::coro::CoroTask<> {
        while (!stop->load()) {
            co_await sleep->sleepFor(ll::chrono::ticks(20));
            if (stop->load()) {
                break;
            }

            auto iter = impl->mSelectors.begin();
            while (iter != impl->mSelectors.end()) {
                auto& selector = iter->second;

                try {
                    if (!selector->getPlayer()) {
                        iter = impl->mSelectors.erase(iter); // 玩家下线
                        continue;
                    }

                    selector->tick();

                    ++iter;
                } catch (std::exception const& e) {
                    iter = impl->mSelectors.erase(iter);
                    land::PLand::getInstance().getSelf().getLogger().error(
                        "SelectorManager: Exception in selector tick: {}",
                        e.what()
                    );
                } catch (...) {
                    iter = impl->mSelectors.erase(iter);
                    land::PLand::getInstance().getSelf().getLogger().error(
                        "SelectorManager: Unknown exception in selector tick"
                    );
                }
            }
        }
        co_return;
    }).launch(ll::thread::ServerThreadExecutor::getDefault());
}

SelectorManager::~SelectorManager() {
    ll::event::EventBus::getInstance().removeListener(impl->mListener);
    ll::event::EventBus::getInstance().removeListener(impl->mPlayerDisconnectListener);

    impl->mSelectors.clear();
    impl->mCoroStop->store(true);
    impl->mInterruptableSleep->interrupt(true);
}


bool SelectorManager::hasSelector(mce::UUID const& uuid) const { return impl->mSelectors.contains(uuid); }
bool SelectorManager::hasSelector(Player& player) const { return impl->mSelectors.contains(player.getUuid()); }

ISelector* SelectorManager::getSelector(mce::UUID const& uuid) const {
    if (auto it = impl->mSelectors.find(uuid); it != impl->mSelectors.end()) {
        return it->second.get();
    }
    return nullptr;
}
ISelector* SelectorManager::getSelector(Player& player) const { return getSelector(player.getUuid()); }

bool SelectorManager::_startSelection(std::unique_ptr<ISelector> selector) {
    auto& uuid = selector->getPlayer()->getUuid();
    if (hasSelector(uuid)) {
        return false;
    }
    return impl->mSelectors.insert({uuid, std::move(selector)}).second;
}

void SelectorManager::stopSelection(mce::UUID const& uuid) {
    if (auto it = impl->mSelectors.find(uuid); it != impl->mSelectors.end()) {
        impl->mSelectors.erase(it);
    }
}
void SelectorManager::stopSelection(Player& player) { return stopSelection(player.getUuid()); }

void SelectorManager::forEach(ForEachFunc const& func) const {
    for (auto const& [uuid, selector] : impl->mSelectors) {
        bool isContinue = func(uuid, selector.get());
        if (!isContinue) {
            break;
        }
    }
}

} // namespace land