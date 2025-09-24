
#include "mc/world/actor/ActorType.h"
#include "pland/hooks/EventListener.h"
#include "pland/hooks/listeners/ListenerHelper.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/entity/ActorHurtEvent.h"
#include "ll/api/event/world/SpawnMobEvent.h"

#include "mc/server/ServerPlayer.h"
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
            if (!mob.has_value()) return;
            auto& pos = mob->getPosition();
            logger->debug("[SpawnedMob] {}", pos.toString());
            auto land = db->getLandAt(pos, mob->getDimensionId());
            if (PreCheckLandExistsAndPermission(land)) return;
            auto const& tab       = land->getPermTable();
            bool        isMonster = mob->hasCategory(::ActorCategory::Monster) || mob->hasFamily("monster");
            if (isMonster) {
                if (!tab.allowMonsterSpawn) mob->despawn();
            } else {
                if (!tab.allowAnimalSpawn) mob->despawn();
            }
        });
    });

    RegisterListenerIf(Config::cfg.listeners.ActorHurtEvent, [&]() {
        return bus->emplaceListener<ll::event::ActorHurtEvent>([db](ll::event::ActorHurtEvent& ev) {
            auto& actor  = ev.self();
            auto& source = ev.source();

            bool const isPlayerDamage = source.getEntityType() == ActorType::Player;
            if (!isPlayerDamage) {
                return; // skip non-player damage
            }

            auto sourcePlayer = actor.getLevel().getPlayer(source.getEntityUniqueID());
            if (!sourcePlayer) {
                return;
            }

            auto land = db->getLandAt(actor.getPosition(), actor.getDimensionId());
            if (PreCheckLandExistsAndPermission(land, sourcePlayer->getUuid())) return;

            auto const& typeName = actor.getTypeName();
            auto const& tab      = land->getPermTable();

            if (isPlayerDamage) {
                CANCEL_AND_RETURN_IF(!tab.allowPlayerDamage);
            } else if (Config::cfg.protection.mob.hostileMobTypeNames.contains(typeName)) {
                CANCEL_AND_RETURN_IF(!tab.allowMonsterDamage);
            } else if (Config::cfg.protection.mob.specialMobTypeNames.contains(typeName)) {
                CANCEL_AND_RETURN_IF(!tab.allowSpecialDamage);
            } else if (Config::cfg.protection.mob.passiveMobTypeNames.contains(typeName)) {
                CANCEL_AND_RETURN_IF(!tab.allowPassiveDamage);
            } else if (Config::cfg.protection.mob.customSpecialMobTypeNames.contains(typeName)) {
                CANCEL_AND_RETURN_IF(!tab.allowCustomSpecialDamage);
            }
        });
    });
}

} // namespace land
