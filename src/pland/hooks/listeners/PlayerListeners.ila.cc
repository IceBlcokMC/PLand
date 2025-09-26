
#include "pland/PLand.h"
#include "pland/hooks/EventListener.h"
#include "pland/hooks/listeners/ListenerHelper.h"
#include "pland/infra/Config.h"
#include "pland/land/LandRegistry.h"

#include "ll/api/event/EventBus.h"

#include "ila/event/minecraft/world/actor/ArmorStandSwapItemEvent.h"
#include "ila/event/minecraft/world/actor/player/PlayerAttackBlockEvent.h"
#include "ila/event/minecraft/world/actor/player/PlayerDropItemEvent.h"
#include "ila/event/minecraft/world/actor/player/PlayerEditSignEvent.h"
#include "ila/event/minecraft/world/actor/player/PlayerInteractEntityEvent.h"
#include "ila/event/minecraft/world/actor/player/PlayerOperatedItemFrameEvent.h"

#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"


namespace land {

void EventListener::registerILAPlayerListeners() {
    auto* db     = PLand::getInstance().getLandRegistry();
    auto* bus    = &ll::event::EventBus::getInstance();
    auto* logger = &land::PLand::getInstance().getSelf().getLogger();

    RegisterListenerIf(Config::cfg.listeners.PlayerInteractEntityBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::PlayerInteractEntityBeforeEvent>(
            [db, logger](ila::mc::PlayerInteractEntityBeforeEvent& ev) {
                auto& player = ev.self();
                auto& target = ev.target();

                EVENT_TRACE(
                    "PlayerInteractEntityEvent",
                    EVENT_TRACE_LOG,
                    "player={}, target={}",
                    player.getRealName(),
                    target.getTypeName()
                );

                auto land = db->getLandAt(target.getPosition(), player.getDimensionId());
                if (PreCheckLandExistsAndPermission(land, player.getUuid())) {
                    EVENT_TRACE("PlayerInteractEntityEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                    return;
                }

                if (land->getPermTable().allowInteractEntity) {
                    EVENT_TRACE("PlayerInteractEntityEvent", EVENT_TRACE_PASS, "allowInteractEntity allowed");
                    return;
                }

                ev.cancel();
                EVENT_TRACE("PlayerInteractEntityEvent", EVENT_TRACE_CANCEL, "permission denied");
            }
        );
    });

    RegisterListenerIf(Config::cfg.listeners.PlayerAttackBlockBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::PlayerAttackBlockBeforeEvent>(
            [db, logger](ila::mc::PlayerAttackBlockBeforeEvent& ev) {
                auto& player = ev.self();
                auto& pos    = ev.pos();

                EVENT_TRACE(
                    "PlayerAttackBlockEvent",
                    EVENT_TRACE_LOG,
                    "player={}, pos={}",
                    player.getRealName(),
                    pos.toString()
                );

                auto land = db->getLandAt(pos, player.getDimensionId());
                if (PreCheckLandExistsAndPermission(land, player.getUuid())) {
                    EVENT_TRACE("PlayerAttackBlockEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                    return;
                }

                auto const& typeName = player.getDimensionBlockSourceConst().getBlock(pos).getTypeName();
                if (typeName == "minecraft:dragon_egg" && !land->getPermTable().allowAttackDragonEgg) {
                    ev.cancel();
                    EVENT_TRACE("PlayerAttackBlockEvent", EVENT_TRACE_CANCEL, "allowAttackDragonEgg denied");
                }
            }
        );
    });

    RegisterListenerIf(Config::cfg.listeners.ArmorStandSwapItemBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::ArmorStandSwapItemBeforeEvent>(
            [db, logger](ila::mc::ArmorStandSwapItemBeforeEvent& ev) {
                auto& player = ev.player();

                EVENT_TRACE("ArmorStandSwapItemEvent", EVENT_TRACE_LOG, "player={}", player.getRealName());

                auto land = db->getLandAt(ev.self().getPosition(), player.getDimensionId());
                if (PreCheckLandExistsAndPermission(land, player.getUuid())) {
                    EVENT_TRACE("ArmorStandSwapItemEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                    return;
                }

                if (land->getPermTable().useArmorStand) {
                    EVENT_TRACE("ArmorStandSwapItemEvent", EVENT_TRACE_PASS, "useArmorStand allowed");
                    return;
                }

                ev.cancel();
                EVENT_TRACE("ArmorStandSwapItemEvent", EVENT_TRACE_CANCEL, "permission denied");
            }
        );
    });

    RegisterListenerIf(Config::cfg.listeners.PlayerDropItemBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::PlayerDropItemBeforeEvent>(
            [db, logger](ila::mc::PlayerDropItemBeforeEvent& ev) {
                auto& player = ev.self();

                EVENT_TRACE("PlayerDropItemEvent", EVENT_TRACE_LOG, "player={}", player.getRealName());

                auto land = db->getLandAt(player.getPosition(), player.getDimensionId());
                if (PreCheckLandExistsAndPermission(land, player.getUuid())) {
                    EVENT_TRACE("PlayerDropItemEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                    return;
                }

                if (land->getPermTable().allowDropItem) {
                    EVENT_TRACE("PlayerDropItemEvent", EVENT_TRACE_PASS, "allowDropItem allowed");
                    return;
                }

                ev.cancel();
                EVENT_TRACE("PlayerDropItemEvent", EVENT_TRACE_CANCEL, "permission denied");
            }
        );
    });

    RegisterListenerIf(Config::cfg.listeners.PlayerOperatedItemFrameBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::PlayerOperatedItemFrameBeforeEvent>(
            [db, logger](ila::mc::PlayerOperatedItemFrameBeforeEvent& ev) {
                auto& player = ev.self();
                auto& pos    = ev.blockPos();

                EVENT_TRACE(
                    "PlayerUseItemFrameEvent",
                    EVENT_TRACE_LOG,
                    "player={}, pos={}",
                    player.getRealName(),
                    pos.toString()
                );

                auto land = db->getLandAt(ev.blockPos(), player.getDimensionId());
                if (PreCheckLandExistsAndPermission(land, player.getUuid())) {
                    EVENT_TRACE("PlayerUseItemFrameEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                    return;
                }

                if (land->getPermTable().useItemFrame) {
                    EVENT_TRACE("PlayerUseItemFrameEvent", EVENT_TRACE_PASS, "useItemFrame allowed");
                    return;
                }

                ev.cancel();
                EVENT_TRACE("PlayerUseItemFrameEvent", EVENT_TRACE_CANCEL, "permission denied");
            }
        );
    });

    RegisterListenerIf(Config::cfg.listeners.PlayerEditSignBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::PlayerEditSignBeforeEvent>(
            [db, logger](ila::mc::PlayerEditSignBeforeEvent& ev) {
                auto& player = ev.self();
                auto& pos    = ev.pos();

                EVENT_TRACE(
                    "PlayerEditSignEvent",
                    EVENT_TRACE_LOG,
                    "player={}, pos={}",
                    player.getRealName(),
                    pos.toString()
                );

                auto land = db->getLandAt(pos, player.getDimensionId());
                if (PreCheckLandExistsAndPermission(land, player.getUuid())) {
                    EVENT_TRACE("PlayerEditSignEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                    return;
                }

                if (!land->getPermTable().editSign) {
                    ev.cancel();
                    EVENT_TRACE("PlayerEditSignEvent", EVENT_TRACE_CANCEL, "editSign denied");
                }
            }
        );
    });
}

} // namespace land
