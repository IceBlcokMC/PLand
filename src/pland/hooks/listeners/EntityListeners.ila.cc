#include "pland/PLand.h"
#include "pland/hooks/EventListener.h"
#include "pland/hooks/listeners/ListenerHelper.h"
#include "pland/hooks/optimize/HashedTypeName.h"
#include "pland/infra/Config.h"
#include "pland/land/LandRegistry.h"

#include "ll/api/event/EventBus.h"

#include "ila/event/minecraft/world/actor/ActorDestroyBlockEvent.h"
#include "ila/event/minecraft/world/actor/ActorRideEvent.h"
#include "ila/event/minecraft/world/actor/ActorTriggerPressurePlateEvent.h"
#include "ila/event/minecraft/world/actor/MobHurtEffectEvent.h"
#include "ila/event/minecraft/world/actor/MobPlaceBlockEvent.h"
#include "ila/event/minecraft/world/actor/MobTakeBlockEvent.h"
#include "ila/event/minecraft/world/actor/ProjectileCreateEvent.h"


#include "mc/deps/ecs/WeakEntityRef.h"
#include "mc/platform/UUID.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/ActorDefinitionIdentifier.h"


namespace land {

void EventListener::registerILAEntityListeners() {
    auto* db     = PLand::getInstance().getLandRegistry();
    auto* bus    = &ll::event::EventBus::getInstance();
    auto* logger = &land::PLand::getInstance().getSelf().getLogger();

    RegisterListenerIf(Config::cfg.listeners.ActorDestroyBlockEvent, [&]() {
        return bus->emplaceListener<ila::mc::ActorDestroyBlockEvent>([db, logger](ila::mc::ActorDestroyBlockEvent& ev) {
            auto& actor    = ev.self();
            auto& blockPos = ev.pos();

            EVENT_TRACE(
                "ActorDestroyBlockEvent",
                EVENT_TRACE_LOG,
                "actor={}, pos={}",
                actor.getTypeName(),
                blockPos.toString()
            );

            auto land = db->getLandAt(blockPos, actor.getDimensionId());
            if (PreCheckLandExistsAndPermission(land)) {
                EVENT_TRACE("ActorDestroyBlockEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                return;
            }

            if (land->getPermTable().allowActorDestroy) {
                EVENT_TRACE("ActorDestroyBlockEvent", EVENT_TRACE_PASS, "allowActorDestroy allowed");
                return;
            }

            ev.cancel();
            EVENT_TRACE("ActorDestroyBlockEvent", EVENT_TRACE_CANCEL, "permission denied");
        });
    });

    RegisterListenerIf(Config::cfg.listeners.MobTakeBlockBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::MobTakeBlockBeforeEvent>([db,
                                                                       logger](ila::mc::MobTakeBlockBeforeEvent& ev) {
            auto& actor    = ev.self();
            auto& blockPos = ev.pos();

            EVENT_TRACE(
                "MobTakeBlockBeforeEvent",
                EVENT_TRACE_LOG,
                "actor={}, pos={}",
                actor.getTypeName(),
                blockPos.toString()
            );

            auto land = db->getLandAt(blockPos, actor.getDimensionId());
            if (PreCheckLandExistsAndPermission(land)) {
                EVENT_TRACE("MobTakeBlockBeforeEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                return;
            }

            if (land->getPermTable().allowActorDestroy) {
                EVENT_TRACE("MobTakeBlockBeforeEvent", EVENT_TRACE_PASS, "allowActorDestroy allowed");
                return;
            }

            ev.cancel();
            EVENT_TRACE("MobTakeBlockBeforeEvent", EVENT_TRACE_CANCEL, "permission denied");
        });
    });

    RegisterListenerIf(Config::cfg.listeners.MobPlaceBlockBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::MobPlaceBlockBeforeEvent>([db,
                                                                        logger](ila::mc::MobPlaceBlockBeforeEvent& ev) {
            auto& actor    = ev.self();
            auto& blockPos = ev.pos();

            EVENT_TRACE(
                "MobPlaceBlockBeforeEvent",
                EVENT_TRACE_LOG,
                "actor={}, pos={}",
                actor.getTypeName(),
                blockPos.toString()
            );

            auto land = db->getLandAt(blockPos, actor.getDimensionId());
            if (PreCheckLandExistsAndPermission(land)) {
                EVENT_TRACE("MobPlaceBlockBeforeEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                return;
            }

            if (land->getPermTable().allowActorDestroy) {
                EVENT_TRACE("MobPlaceBlockBeforeEvent", EVENT_TRACE_PASS, "allowActorDestroy allowed");
                return;
            }

            ev.cancel();
            EVENT_TRACE("MobPlaceBlockBeforeEvent", EVENT_TRACE_CANCEL, "permission denied");
        });
    });

    RegisterListenerIf(Config::cfg.listeners.ActorRideBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::ActorRideBeforeEvent>([db, logger](ila::mc::ActorRideBeforeEvent& ev) {
            Actor& passenger = ev.self();
            Actor& target    = ev.target();

            if (!passenger.isPlayer()) {
                EVENT_TRACE("ActorRideEvent", EVENT_TRACE_SKIP, "passenger is not player");
                return;
            }

            EVENT_TRACE(
                "ActorRideEvent",
                EVENT_TRACE_LOG,
                "passenger: {}, target: {}",
                passenger.getActorIdentifier().mIdentifier.get(),
                target.getTypeName()
            );

            auto& player = static_cast<Player&>(passenger);
            auto  land   = db->getLandAt(target.getPosition(), target.getDimensionId());
            if (PreCheckLandExistsAndPermission(land, player.getUuid())) {
                EVENT_TRACE("ActorRideEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                return;
            }

            auto& typeName       = target.getTypeName();
            auto  hashedTypeName = HashedStringView{typeName};

            auto& tab = land->getPermTable();
            if (hashedTypeName == HashedTypeName::Minecart || hashedTypeName == HashedTypeName::Boat
                || hashedTypeName == HashedTypeName::ChestBoat) {
                if (tab.allowRideTrans) {
                    EVENT_TRACE("ActorRideEvent", EVENT_TRACE_PASS, "allowRideTrans allowed");
                    return;
                }
            } else {
                if (tab.allowRideEntity) {
                    EVENT_TRACE("ActorRideEvent", EVENT_TRACE_PASS, "allowRideEntity allowed");
                    return;
                }
            }

            ev.cancel();
            EVENT_TRACE("ActorRideEvent", EVENT_TRACE_CANCEL, "permission denied");
        });
    });

    RegisterListenerIf(Config::cfg.listeners.MobHurtEffectBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::MobHurtEffectBeforeEvent>([db,
                                                                        logger](ila::mc::MobHurtEffectBeforeEvent& ev) {
            auto& actor = ev.self();

            auto sourceActor = ev.source();
            if (!sourceActor || !sourceActor->isPlayer()) {
                EVENT_TRACE("MobHurtEffectEvent", EVENT_TRACE_SKIP, "source is not player");
                return;
            }

            auto& sourcePlayer = static_cast<Player&>(sourceActor.value());

            auto land = db->getLandAt(actor.getPosition(), actor.getDimensionId());
            if (PreCheckLandExistsAndPermission(land, sourcePlayer.getUuid())) {
                EVENT_TRACE("MobHurtEffectEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                return;
            }

            auto const& typeName = actor.getTypeName();
            auto const& tab      = land->getPermTable();

            if (actor.isPlayer()) {
                CANCEL_AND_RETURN_IF(
                    !tab.allowPlayerDamage,
                    EVENT_TRACE("MobHurtEffectEvent", EVENT_TRACE_CANCEL, "allowPlayerDamage denied")
                );
            } else if (Config::cfg.protection.mob.hostileMobTypeNames.contains(typeName)) {
                CANCEL_AND_RETURN_IF(
                    !tab.allowMonsterDamage,
                    EVENT_TRACE("MobHurtEffectEvent", EVENT_TRACE_CANCEL, "allowMonsterDamage denied")
                );
            } else if (Config::cfg.protection.mob.specialMobTypeNames.contains(typeName)) {
                CANCEL_AND_RETURN_IF(
                    !tab.allowSpecialDamage,
                    EVENT_TRACE("MobHurtEffectEvent", EVENT_TRACE_CANCEL, "allowSpecialDamage denied")
                );
            } else if (Config::cfg.protection.mob.passiveMobTypeNames.contains(typeName)) {
                CANCEL_AND_RETURN_IF(
                    !tab.allowPassiveDamage,
                    EVENT_TRACE("MobHurtEffectEvent", EVENT_TRACE_CANCEL, "allowPassiveDamage denied")
                );
            } else if (Config::cfg.protection.mob.customSpecialMobTypeNames.contains(typeName)) {
                CANCEL_AND_RETURN_IF(
                    !tab.allowCustomSpecialDamage,
                    EVENT_TRACE("MobHurtEffectEvent", EVENT_TRACE_CANCEL, "allowCustomSpecialDamage denied")
                );
            }
        });
    });

    RegisterListenerIf(Config::cfg.listeners.ActorTriggerPressurePlateBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::ActorTriggerPressurePlateBeforeEvent>(
            [db, logger](ila::mc::ActorTriggerPressurePlateBeforeEvent& ev) {
                EVENT_TRACE("ActorTriggerPressurePlateEvent", EVENT_TRACE_LOG, "pos={}", ev.pos().toString());

                auto& actor = ev.self();
                auto& uuid  = actor.isPlayer() ? static_cast<Player&>(actor).getUuid() : mce::UUID::EMPTY();

                auto land = db->getLandAt(ev.pos(), ev.self().getDimensionId());
                if (PreCheckLandExistsAndPermission(land, uuid)) {
                    EVENT_TRACE(
                        "ActorTriggerPressurePlateEvent",
                        EVENT_TRACE_PASS,
                        "land not found or permission allowed"
                    );
                    return;
                }

                if (land->getPermTable().usePressurePlate) {
                    EVENT_TRACE("ActorTriggerPressurePlateEvent", EVENT_TRACE_PASS, "usePressurePlate allowed");
                    return;
                }

                ev.cancel();
                EVENT_TRACE("ActorTriggerPressurePlateEvent", EVENT_TRACE_CANCEL, "permission denied");
            }
        );
    });

    RegisterListenerIf(Config::cfg.listeners.ProjectileCreateBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::ProjectileCreateBeforeEvent>(
            [db, logger](ila::mc::ProjectileCreateBeforeEvent& ev) {
                auto& projectile = ev.self(); // projectile
                auto  owner      = projectile.getOwner();
                if (!owner) {
                    EVENT_TRACE("ProjectileCreateEvent", EVENT_TRACE_SKIP, "projectile has no owner");
                    return;
                }

                auto& typeName = projectile.getTypeName();
                EVENT_TRACE("ProjectileCreateEvent", EVENT_TRACE_LOG, "type={}", typeName);

                auto& uuid = owner->isPlayer() ? static_cast<Player&>(*owner).getUuid() : mce::UUID::EMPTY();
                auto  land = db->getLandAt(projectile.getPosition(), projectile.getDimensionId());
                if (PreCheckLandExistsAndPermission(land, uuid)) {
                    EVENT_TRACE("ProjectileCreateEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                    return;
                }

                auto const& tab = land->getPermTable();
                if (HashedStringView{typeName} == HashedTypeName::FishingHook) {
                    CANCEL_AND_RETURN_IF(
                        !tab.allowFishingRodAndHook,
                        EVENT_TRACE("ProjectileCreateEvent", EVENT_TRACE_CANCEL, "allowFishingRodAndHook denied")
                    );
                } else {
                    CANCEL_AND_RETURN_IF(
                        !tab.allowProjectileCreate,
                        EVENT_TRACE("ProjectileCreateEvent", EVENT_TRACE_CANCEL, "allowProjectileCreate denied")
                    )
                }
            }
        );
    });
}

} // namespace land
