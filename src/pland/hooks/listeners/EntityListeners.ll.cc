#include "pland/hooks/EventListener.h"
#include "pland/hooks/listeners/ListenerHelper.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/entity/ActorHurtEvent.h"
#include "ll/api/event/world/SpawnMobEvent.h"

#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/level/Level.h"

#include "pland/PLand.h"
#include "pland/infra/Config.h"
#include "pland/land/LandRegistry.h"


namespace land {

void EventListener::registerLLEntityListeners() {
    auto* db     = PLand::getInstance().getLandRegistry();
    auto* bus    = &ll::event::EventBus::getInstance();
    auto* logger = &land::PLand::getInstance().getSelf().getLogger();

    RegisterListenerIf(Config::cfg.listeners.SpawnedMobEvent, [&]() {
        return bus->emplaceListener<ll::event::SpawnedMobEvent>([db, logger](ll::event::SpawnedMobEvent& ev) {
            auto mob = ev.mob();
            if (!mob.has_value()) {
                EVENT_TRACE("SpawnedMobEvent", EVENT_TRACE_SKIP, "mob not found");
                return;
            }
            auto& pos = mob->getPosition();
            EVENT_TRACE("SpawnedMobEvent", EVENT_TRACE_LOG, "position={}", pos.toString());

            auto land = db->getLandAt(pos, mob->getDimensionId());
            if (PreCheckLandExistsAndPermission(land)) {
                EVENT_TRACE("SpawnedMobEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                return;
            }

            auto const& tab       = land->getPermTable();
            bool const  isMonster = mob->hasCategory(::ActorCategory::Monster) || mob->hasFamily("monster");
            if ((isMonster && !tab.allowMonsterSpawn) || (!isMonster && !tab.allowAnimalSpawn)) {
                mob->despawn();
                EVENT_TRACE("SpawnedMobEvent", EVENT_TRACE_CANCEL, "mob despawned");
            }
        });
    });

    RegisterListenerIf(Config::cfg.listeners.ActorHurtEvent, [&]() {
        return bus->emplaceListener<ll::event::ActorHurtEvent>([db, logger](ll::event::ActorHurtEvent& ev) {
            auto& actor  = ev.self();
            auto& source = ev.source();

            // MobHurtHook 已经处理了非玩家伤害的领地保护，这里只处理玩家伤害
            auto sourcePlayer = actor.getLevel().getPlayer(source.getEntityUniqueID());
            if (!sourcePlayer) {
                EVENT_TRACE("ActorHurtEvent", EVENT_TRACE_PASS, "source player not found");
                return;
            }

            auto land = db->getLandAt(actor.getPosition(), actor.getDimensionId());
            if (PreCheckLandExistsAndPermission(land, sourcePlayer->getUuid())) {
                EVENT_TRACE("ActorHurtEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                return;
            }

            auto const& typeName = actor.getTypeName();
            auto const& tab      = land->getPermTable();

            if (actor.isPlayer()) {
                CANCEL_AND_RETURN_IF(
                    !tab.allowPlayerDamage,
                    EVENT_TRACE("ActorHurtEvent", EVENT_TRACE_CANCEL, "allowPlayerDamage denied")
                );
            } else if (Config::cfg.protection.mob.hostileMobTypeNames.contains(typeName)) {
                CANCEL_AND_RETURN_IF(
                    !tab.allowMonsterDamage,
                    EVENT_TRACE("ActorHurtEvent", EVENT_TRACE_CANCEL, "allowMonsterDamage denied")
                );
            } else if (Config::cfg.protection.mob.specialMobTypeNames.contains(typeName)) {
                CANCEL_AND_RETURN_IF(
                    !tab.allowSpecialDamage,
                    EVENT_TRACE("ActorHurtEvent", EVENT_TRACE_CANCEL, "allowSpecialDamage denied")
                );
            } else if (Config::cfg.protection.mob.passiveMobTypeNames.contains(typeName)) {
                CANCEL_AND_RETURN_IF(
                    !tab.allowPassiveDamage,
                    EVENT_TRACE("ActorHurtEvent", EVENT_TRACE_CANCEL, "allowPassiveDamage denied")
                );
            } else if (Config::cfg.protection.mob.customSpecialMobTypeNames.contains(typeName)) {
                CANCEL_AND_RETURN_IF(
                    !tab.allowCustomSpecialDamage,
                    EVENT_TRACE("ActorHurtEvent", EVENT_TRACE_CANCEL, "allowCustomSpecialDamage denied")
                );
            }
        });
    });
}

} // namespace land
