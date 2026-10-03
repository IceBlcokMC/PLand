#include "LandScheduler.h"

#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/coro/InterruptableSleep.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/event/player/PlayerDisconnectEvent.h"
#include "ll/api/event/player/PlayerJoinEvent.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/service/PlayerInfo.h"
#include "ll/api/thread/ServerThreadExecutor.h"


#include "mc/network/packet/SetTitlePacket.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"

#include "pland/Global.h"
#include "pland/PLand.h"
#include "pland/events/player/PlayerMoveEvent.h"
#include "pland/land/Config.h"
#include "pland/land/Land.h"
#include "pland/land/repo/LandRegistry.h"
#include "pland/land/repo/PlayerSettings.h"

#include <vector>

#include "absl/container/flat_hash_map.h"

namespace land::internal {

// TODO: 重构解耦此模块
// 按通用调度系统处理进出领地事件和状态管理 + ILandTickSubSystem 处理子任务
// 持久化消息提示等，移入例如 LandActionBarTipSubSystem 系统处理，而非一个类处理全部逻辑

struct LandScheduler::Impl {
    std::vector<Player*>                    mPlayers{};
    absl::flat_hash_map<Player*, LandDimid> mDimensionMap{};
    absl::flat_hash_map<Player*, LandID>    mLandIdMap{};

    ll::event::ListenerPtr mPlayerJoinServerListener{nullptr};
    ll::event::ListenerPtr mPlayerDisconnectListener{nullptr};
    ll::event::ListenerPtr mPlayerEnterLandListener{nullptr};

    std::shared_ptr<std::atomic<bool>>            mQuit{nullptr};
    std::shared_ptr<ll::coro::InterruptableSleep> mEventSchedulingSleep{nullptr};
    std::shared_ptr<ll::coro::InterruptableSleep> mLandTipSchedulingSleep{nullptr};

    void tickEvent() {
        auto& bus      = ll::event::EventBus::getInstance();
        auto& registry = PLand::getInstance().getLandRegistry();

        auto iter = mPlayers.begin();
        while (iter != mPlayers.end()) {
            try {
                auto player = *iter;

                auto const& currentPos   = player->getPosition();
                int const   currentDimId = player->getDimensionId();

                int&  lastDimId  = mDimensionMap[player];
                auto& lastLandID = mLandIdMap[player];

                auto   land          = registry.getLandAt(currentPos, currentDimId);
                LandID currentLandId = land ? land->getId() : INVALID_LAND_ID;

                // 处理维度变化
                if (currentDimId != lastDimId) {
                    if (lastLandID != INVALID_LAND_ID) {
                        bus.publish(event::PlayerLeaveLandEvent{*player, lastLandID}); // 离开上一个维度的领地
                    }
                    lastDimId = currentDimId;
                }

                // 处理领地变化
                if (currentLandId != lastLandID) {
                    if (lastLandID != INVALID_LAND_ID) {
                        bus.publish(event::PlayerLeaveLandEvent{*player, lastLandID}); // 离开上一个领地
                    }
                    if (currentLandId != INVALID_LAND_ID) {
                        bus.publish(event::PlayerEnterLandEvent{*player, currentLandId}); // 进入新领地
                    }
                    lastLandID = currentLandId;
                }
                ++iter;
            } catch (...) {
                iter = mPlayers.erase(iter);
            }
        }
    }

    void tickLandTip() {
        auto& playerInfo = ll::service::PlayerInfo::getInstance();
        auto& registry   = PLand::getInstance().getLandRegistry();

        SetTitlePacket pkt{
            SetTitlePacketPayload{SetTitlePacketPayload::TitleType::Actionbar, "", std::nullopt}
        };
        pkt.mType = SetTitlePacket::TitleType::Actionbar;
        for (auto& [player, landId] : mLandIdMap) {
            if (landId == INVALID_LAND_ID) {
                continue;
            }

            if (!registry.getOrCreatePlayerSettings(player->getUuid()).showBottomContinuedTip) {
                continue; // 如果玩家设置不显示底部提示，则跳过
            }

            auto land = registry.getLand(landId);
            if (!land) {
                continue;
            }

            auto localeCode = player->getLocaleCode();
            switch (land->getOwnershipKind()) {
            case LandOwnershipKind::Player: {
                if (land->isOwner(player->getUuid())) {
                    pkt.mTitleText = "{} - 我的领地"_trl(localeCode, land->getName());
                    break;
                }
                auto const& owner     = land->getOwner();
                auto        ownerInfo = playerInfo.fromUuid(owner);
                pkt.mTitleText        = "{} - 所有者: {}"_trl(
                    localeCode,
                    land->getName(),
                    ownerInfo.has_value() ? ownerInfo->name : owner.asString()
                );
                break;
            }
            case LandOwnershipKind::System:
                pkt.mTitleText = "{} - 系统托管"_trl(localeCode, land->getName());
                break;
            case LandOwnershipKind::Ownerless:
                pkt.mTitleText = "{} - 无主领地"_trl(localeCode, land->getName());
                break;
            case LandOwnershipKind::PendingMigration:
                pkt.mTitleText = "{} - 旧版待迁移"_trl(localeCode, land->getName());
                break;
            }

            pkt.sendTo(*player);
        }
    }
};


LandScheduler::LandScheduler() : impl(std::make_unique<Impl>()) {
    auto& bus = ll::event::EventBus::getInstance();

    impl->mQuit                   = std::make_shared<std::atomic<bool>>(false);
    impl->mEventSchedulingSleep   = std::make_shared<ll::coro::InterruptableSleep>();
    impl->mLandTipSchedulingSleep = std::make_shared<ll::coro::InterruptableSleep>();

    impl->mPlayerJoinServerListener =
        bus.emplaceListener<ll::event::PlayerJoinEvent>([this](ll::event::PlayerJoinEvent& ev) {
            auto& player = ev.self();
            if (player.isSimulatedPlayer()) {
                return;
            }
            impl->mPlayers.emplace_back(&player);
        });

    impl->mPlayerDisconnectListener =
        bus.emplaceListener<ll::event::PlayerDisconnectEvent>([this](ll::event::PlayerDisconnectEvent& ev) {
            auto& player = ev.self();
            if (player.isSimulatedPlayer()) {
                return;
            }

            auto ptr = &player;
            impl->mDimensionMap.erase(ptr);
            impl->mLandIdMap.erase(ptr);
            std::erase_if(impl->mPlayers, [&ptr](auto* p) { return p == ptr; });
        });

    impl->mPlayerEnterLandListener =
        bus.emplaceListener<event::PlayerEnterLandEvent>([](event::PlayerEnterLandEvent& ev) {
            auto const& conf = ConfigProvider::getNotificationsConfig();
            if (!conf.enterLandTip) {
                return;
            }

            auto& player   = ev.self();
            auto& registry = PLand::getInstance().getLandRegistry();

            if (!registry.getOrCreatePlayerSettings(player.getUuid()).showEnterLandTitle) {
                return; // 如果玩家设置不显示进入领地提示,则不显示
            }

            auto land = registry.getLand(ev.landId());
            if (!land) {
                return;
            }

            SetTitlePacket title{
                SetTitlePacketPayload{SetTitlePacketPayload::TitleType::Title, "", std::nullopt}
            };
            SetTitlePacket subTitle{
                SetTitlePacketPayload{SetTitlePacketPayload::TitleType::Subtitle, "", std::nullopt}
            };
            title.mType    = SetTitlePacket::TitleType::Title;
            subTitle.mType = SetTitlePacket::TitleType::Subtitle;

            auto localeCode = player.getLocaleCode();

            title.mTitleText = land->getName();
            switch (land->getOwnershipKind()) {
            case LandOwnershipKind::Player: {
                if (land->isOwner(player.getUuid())) {
                    subTitle.mTitleText = "欢迎回家"_trl(localeCode);
                    break;
                }
                auto ownerInfo = ll::service::PlayerInfo::getInstance().fromUuid(land->getOwner());
                subTitle.mTitleText =
                    "所有者: {}"_trl(localeCode, ownerInfo.has_value() ? ownerInfo->name : land->getOwner().asString());
                break;
            }
            case LandOwnershipKind::System:
                subTitle.mTitleText = "系统托管"_trl(localeCode);
                break;
            case LandOwnershipKind::Ownerless:
                subTitle.mTitleText = "无主领地"_trl(localeCode);
                break;
            case LandOwnershipKind::PendingMigration:
                subTitle.mTitleText = "旧版领地"_trl(localeCode);
                break;
            }

            title.sendTo(player);
            subTitle.sendTo(player);
        });

    ll::coro::keepThis([quit = impl->mQuit, sleep = impl->mEventSchedulingSleep, this]() -> ll::coro::CoroTask<> {
        while (!quit->load()) {
            co_await sleep->sleepFor(ll::chrono::ticks{5});
            if (quit->load()) {
                break;
            }

            if (impl->mPlayers.empty()) {
                continue;
            }

            try {
                impl->tickEvent();
            } catch (std::exception& e) {
                PLand::getInstance().getSelf().getLogger().error(
                    "An exception occurred while scheduling land events: {}",
                    e.what()
                );
            } catch (...) {
                PLand::getInstance().getSelf().getLogger().error(
                    "An unknown exception occurred while scheduling land events."
                );
            }
        }
        co_return;
    }).launch(ll::thread::ServerThreadExecutor::getDefault());

    auto& conf = ConfigProvider::getNotificationsConfig();
    if (conf.bottomContinuousTip) {
        ll::coro::keepThis([quit = impl->mQuit, sleep = impl->mLandTipSchedulingSleep, this]() -> ll::coro::CoroTask<> {
            while (!quit->load()) {
                co_await sleep->sleepFor(
                    ConfigProvider::getNotificationsConfig().bottomTipCycle * ll::chrono::ticks{20}
                );
                if (quit->load()) {
                    break;
                }

                if (impl->mLandIdMap.empty()) {
                    continue;
                }

                try {
                    impl->tickLandTip();
                } catch (std::exception& e) {
                    PLand::getInstance().getSelf().getLogger().error(
                        "An exception occurred while scheduling land tip: {}",
                        e.what()
                    );
                } catch (...) {
                    PLand::getInstance().getSelf().getLogger().error(
                        "An unknown exception occurred while scheduling land tip."
                    );
                }
            }
        }).launch(ll::thread::ServerThreadExecutor::getDefault());
    }
}

LandScheduler::~LandScheduler() {
    auto& bus = ll::event::EventBus::getInstance();
    bus.removeListener(impl->mPlayerEnterLandListener);
    bus.removeListener(impl->mPlayerJoinServerListener);
    bus.removeListener(impl->mPlayerDisconnectListener);
    impl->mQuit->store(true);
    impl->mEventSchedulingSleep->interrupt(true);
    impl->mLandTipSchedulingSleep->interrupt(true);
    impl->mPlayers.clear();
    impl->mDimensionMap.clear();
    impl->mLandIdMap.clear();
}


} // namespace land::internal
