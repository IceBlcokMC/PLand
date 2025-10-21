
#include "pland/hooks/EventListener.h"
#include "pland/hooks/listeners/ListenerHelper.h"

#include "ll/api/event/EventBus.h"

#include "ila/event/minecraft/world/ExplosionEvent.h"
#include "ila/event/minecraft/world/PistonPushEvent.h"
#include "ila/event/minecraft/world/RedstoneUpdateEvent.h"
#include "ila/event/minecraft/world/SculkBlockGrowthEvent.h"
#include "ila/event/minecraft/world/WitherDestroyEvent.h"
#include "ila/event/minecraft/world/level/block/BlockFallEvent.h"
#include "ila/event/minecraft/world/level/block/DragonEggBlockTeleportEvent.h"
#include "ila/event/minecraft/world/level/block/FarmDecayEvent.h"
#include "ila/event/minecraft/world/level/block/LiquidFlowEvent.h"
#include "ila/event/minecraft/world/level/block/MossGrowthEvent.h"
#include "ila/event/minecraft/world/level/block/SculkCatalystAbsorbExperienceEvent.h"
#include "ila/event/minecraft/world/level/block/SculkSpreadEvent.h"

#include "mc/world/level/Explosion.h"
#include "mc/world/phys/AABB.h"

#include "pland/PLand.h"
#include "pland/infra/Config.h"
#include "pland/land/LandRegistry.h"

namespace land {

void EventListener::registerILAWorldListeners() {
    auto* db     = PLand::getInstance().getLandRegistry();
    auto* bus    = &ll::event::EventBus::getInstance();
    auto* logger = &land::PLand::getInstance().getSelf().getLogger();


    RegisterListenerIf(Config::cfg.listeners.ExplosionBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::ExplosionBeforeEvent>([db, logger](ila::mc::ExplosionBeforeEvent& ev) {
            auto       explosionPos = BlockPos{ev.explosion().mPos};
            auto       dimid        = ev.blockSource().getDimensionId();
            SharedLand centerLand   = db->getLandAt(explosionPos, dimid);

            EVENT_TRACE("ExplosionEvent", EVENT_TRACE_LOG, "pos={}", explosionPos.toString());

            if (centerLand) {
                // 规则一：爆炸中心所在领地的权限具有决定性。
                if (!centerLand->getPermTable().allowExplode) {
                    EVENT_TRACE("ExplosionEvent", EVENT_TRACE_CANCEL, "center land does not allow explode");
                    ev.cancel();
                    return;
                }

                // 规则二：如果中心领地允许爆炸，检查是否影响到其他禁止爆炸的、不相关的领地。
                auto touchedLands = db->getLandAt(explosionPos, (int)(ev.explosion().mRadius + 1.0), dimid);
                auto centerRoot   = centerLand->getRootLand();
                for (auto const& touchedLand : touchedLands) {
                    if (touchedLand->getRootLand() != centerRoot) {
                        if (!touchedLand->getPermTable().allowExplode) {
                            EVENT_TRACE("ExplosionEvent", EVENT_TRACE_CANCEL, "touched land does not allow explode");
                            ev.cancel();
                            return;
                        }
                    }
                }
            } else {
                // 情况：爆炸发生在领地外。
                // 如果影响到任何禁止爆炸的领地，则取消。
                auto touchedLands = db->getLandAt(explosionPos, (int)(ev.explosion().mRadius + 1.0), dimid);
                for (auto const& touchedLand : touchedLands) {
                    if (!touchedLand->getPermTable().allowExplode) {
                        EVENT_TRACE("ExplosionEvent", EVENT_TRACE_CANCEL, "external land does not allow explode");
                        ev.cancel();
                        return;
                    }
                }
            }
        });
    });


    RegisterListenerIf(Config::cfg.listeners.FarmDecayBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::FarmDecayBeforeEvent>([db, logger](ila::mc::FarmDecayBeforeEvent& ev) {
            auto& pos = ev.pos();

            EVENT_TRACE("FarmDecayEvent", EVENT_TRACE_LOG, "pos={}", pos.toString());

            auto land = db->getLandAt(pos, ev.blockSource().getDimensionId());
            if (PreCheckLandExistsAndPermission(land) || land->getPermTable().allowFarmDecay) {
                EVENT_TRACE("FarmDecayEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                return;
            }

            ev.cancel();
            EVENT_TRACE("FarmDecayEvent", EVENT_TRACE_CANCEL, "permission denied");
        });
    });

    RegisterListenerIf(Config::cfg.listeners.PistonPushBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::PistonPushBeforeEvent>([db](ila::mc::PistonPushBeforeEvent& ev) {
            auto const& piston     = ev.pistonPos();
            auto const& push       = ev.pushPos();
            auto const  dimid      = ev.blockSource().getDimensionId();
            auto        pistonLand = db->getLandAt(piston, dimid);
            auto        pushLand   = db->getLandAt(push, dimid);
            if (pistonLand && pushLand) {
                if (pistonLand == pushLand
                    || (pistonLand->getPermTable().allowPistonPushOnBoundary
                        && pushLand->getPermTable().allowPistonPushOnBoundary)) {
                    return;
                }
                ev.cancel();
            } else if (!pistonLand && pushLand) {
                if (!pushLand->getPermTable().allowPistonPushOnBoundary
                    && (pushLand->getAABB().isOnOuterBoundary(piston) || pushLand->getAABB().isOnInnerBoundary(push))) {
                    ev.cancel();
                }
            }
        });
    });

    RegisterListenerIf(Config::cfg.listeners.RedstoneUpdateBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::RedstoneUpdateBeforeEvent>([db](ila::mc::RedstoneUpdateBeforeEvent& ev) {
            auto land = db->getLandAt(ev.pos(), ev.blockSource().getDimensionId());
            if (PreCheckLandExistsAndPermission(land) || (land && land->getPermTable().allowRedstoneUpdate)) return;
            ev.cancel();
        });
    });

    RegisterListenerIf(Config::cfg.listeners.BlockFallBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::BlockFallBeforeEvent>([db](ila::mc::BlockFallBeforeEvent& ev) {
            auto land = db->getLandAt(ev.pos(), ev.blockSource().getDimensionId());
            if (land) {
                auto const& tab = land->getPermTable();
                if (land->getAABB().isAboveLand(ev.pos()) && !tab.allowBlockFall) {
                    ev.cancel();
                }
            }
        });
    });

    RegisterListenerIf(Config::cfg.listeners.WitherDestroyBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::WitherDestroyBeforeEvent>([db,
                                                                        logger](ila::mc::WitherDestroyBeforeEvent& ev) {
            auto& aabb = ev.box();

            EVENT_TRACE("WitherDestroyEvent", EVENT_TRACE_LOG, "aabb={}", aabb.toString());

            static constexpr float Offset = 1.0f; // 由于闭区间判定以及浮点数精度，需要额外偏移1个单位

            auto lands = db->getLandAt(aabb.min - Offset, aabb.max + Offset, ev.blockSource().getDimensionId());
            for (auto const& p : lands) {
                if (!p->getPermTable().allowWitherDestroy) {
                    EVENT_TRACE("WitherDestroyEvent", EVENT_TRACE_CANCEL, "allowWitherDestroy denied");
                    ev.cancel();
                    break;
                }
            }
        });
    });

    RegisterListenerIf(Config::cfg.listeners.MossGrowthBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::MossGrowthBeforeEvent>([db](ila::mc::MossGrowthBeforeEvent& ev) {
            auto const& pos  = ev.pos();
            auto        land = db->getLandAt(pos, ev.blockSource().getDimensionId());
            if (!land || land->getPermTable().useBoneMeal) return;
            auto lds = db->getLandAt(pos - 9, pos + 9, ev.blockSource().getDimensionId());
            for (auto const& p : lds) {
                if (p->getPermTable().useBoneMeal) return;
            }
            ev.cancel();
        });
    });

    RegisterListenerIf(Config::cfg.listeners.LiquidFlowBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::LiquidFlowBeforeEvent>([db](ila::mc::LiquidFlowBeforeEvent& ev) {
            auto& sou    = ev.flowFromPos();
            auto& to     = ev.pos();
            auto  landTo = db->getLandAt(to, ev.blockSource().getDimensionId());
            if (landTo && !landTo->getPermTable().allowLiquidFlow && landTo->getAABB().isOnOuterBoundary(sou)
                && landTo->getAABB().isOnInnerBoundary(to)) {
                ev.cancel();
            }
        });
    });

    RegisterListenerIf(Config::cfg.listeners.DragonEggBlockTeleportBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::DragonEggBlockTeleportBeforeEvent>(
            [db](ila::mc::DragonEggBlockTeleportBeforeEvent& ev) {
                auto land = db->getLandAt(ev.pos(), ev.blockSource().getDimensionId());
                if (land && !land->getPermTable().allowAttackDragonEgg) {
                    ev.cancel();
                }
            }
        );
    });

    RegisterListenerIf(Config::cfg.listeners.SculkBlockGrowthBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::SculkBlockGrowthBeforeEvent>(
            [db](ila::mc::SculkBlockGrowthBeforeEvent& ev) {
                auto land = db->getLandAt(ev.pos(), ev.blockSource().getDimensionId());
                if (land && !land->getPermTable().allowSculkBlockGrowth) {
                    ev.cancel();
                }
            }
        );
    });

    RegisterListenerIf(Config::cfg.listeners.SculkSpreadBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::SculkSpreadBeforeEvent>([db](ila::mc::SculkSpreadBeforeEvent& ev) {
            auto sou = db->getLandAt(ev.selfPos(), ev.blockSource().getDimensionId());
            auto tar = db->getLandAt(ev.targetPos(), ev.blockSource().getDimensionId());
            if (!sou && tar) {
                ev.cancel();
            }
        });
    });

    RegisterListenerIf(Config::cfg.listeners.SculkCatalystAbsorbExperienceBeforeEvent, [&]() {
        return bus->emplaceListener<ila::mc::SculkCatalystAbsorbExperienceBeforeEvent>(
            [db](ila::mc::SculkCatalystAbsorbExperienceBeforeEvent& ev) {
                auto& actor  = ev.actor();
                auto& region = actor.getDimensionBlockSource();
                auto  pos    = actor.getBlockPosCurrentlyStandingOn(&actor);
                auto  cur    = db->getLandAt(pos, region.getDimensionId());
                auto  lds    = db->getLandAt(pos - 9, pos + 9, region.getDimensionId());
                if ((cur && lds.size() == 1) || (!cur && lds.empty())) return;
                ev.cancel();
            }
        );
    });
}

} // namespace land
