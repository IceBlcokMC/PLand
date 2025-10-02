
#include "hook.h"
#include "mc/world/level/BlockSource.h"
#include "pland/Global.h"
#include "pland/PLand.h"
#include "pland/hooks/listeners/ListenerHelper.h"
#include "pland/infra/Config.h"
#include "pland/land/LandRegistry.h"


#include "ll/api/memory/Hook.h"

#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/actor/FishingHook.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/ai/goal/LayEggGoal.h"
#include "mc/world/actor/player/Player.h" // 添加 Player 头文件
#include "mc/world/level/Level.h"


namespace land {
// 受伤事件的增强判断
LL_TYPE_INSTANCE_HOOK(
    MobHurtHook,
    HookPriority::Normal,
    Mob,
    &Mob::$_hurt,
    bool,
    ::ActorDamageSource const& source,
    float                      damage,
    bool                       knock,
    bool                       ignite
) {
    auto& logger = land::PLand::getInstance().getSelf().getLogger();
    // 获取受伤生物的位置和维度
    auto& actor = *this;
    auto& pos   = actor.getPosition();
    auto  dimId = actor.getDimensionId();

    // 获取领地注册表实例
    auto* db   = PLand::getInstance().getLandRegistry();
    auto  land = db->getLandAt(pos, dimId);

    // 如果生物在领地内
    if (land) {
        if (source.getEntityType() != ActorType::Player) {
            // 如果伤害来源不是玩家，则取消伤害
            return false;
        } else {
            // 伤害来源是玩家
            auto sourcePlayer = actor.getLevel().getPlayer(source.getEntityUniqueID());
            if (!sourcePlayer) {
                // 找不到来源玩家，允许伤害（或者根据需求取消）
                return origin(source, damage, knock, ignite);
            }

            if (PreCheckLandExistsAndPermission(land, sourcePlayer->getUuid())) {
                // 领地不存在或玩家有权限，允许伤害
                return origin(source, damage, knock, ignite);
            }

            auto const& typeName = actor.getTypeName();
            auto const& tab      = land->getPermTable();

            if (actor.isPlayer()) {
                if (!tab.allowPlayerDamage) {
                    return false;
                }
            } else if (Config::cfg.protection.mob.hostileMobTypeNames.contains(typeName)) {
                if (!tab.allowMonsterDamage) {
                    return false;
                }
            } else if (Config::cfg.protection.mob.specialMobTypeNames.contains(typeName)) {
                if (!tab.allowSpecialDamage) {
                    return false;
                }
            } else if (Config::cfg.protection.mob.passiveMobTypeNames.contains(typeName)) {
                if (!tab.allowPassiveDamage) {
                    return false;
                }
            } else if (Config::cfg.protection.mob.customSpecialMobTypeNames.contains(typeName)) {
                if (!tab.allowCustomSpecialDamage) {
                    return false;
                }
            }
        }
    }

    return origin(source, damage, knock, ignite);
}

LL_TYPE_INSTANCE_HOOK(
    onFishingHookHitHook,
    ll::memory::HookPriority::Highest,
    ::FishingHook,
    &::FishingHook::_pullCloser,
    void,
    Actor& inEntity,
    float  inSpeed
) {
    // 获取钓鱼钩的位置和维度
    auto& hookActor = *this;
    auto& pos       = hookActor.getPosition();
    auto  dimId     = hookActor.getDimensionId();

    // 获取领地注册表实例
    auto* db   = PLand::getInstance().getLandRegistry();
    auto  land = db->getLandAt(pos, dimId);

    // 如果在领地内
    if (land) {
        // 检查 inEntity 是否为玩家
        if (inEntity.isPlayer()) {
            auto& player = static_cast<Player&>(inEntity);
            if (!PreCheckLandExistsAndPermission(land, player.getUuid())) {
                // 领地不存在或玩家没有权限，则拦截
                return;
            }
            // 检查钓鱼竿权限
            if (!land->getPermTable().allowFishingRodAndHook) {
                // 如果不允许使用钓鱼竿，则拦截
                return;
            }
        }
    }
    origin(inEntity, inSpeed);
}

LL_TYPE_INSTANCE_HOOK(
    onLayEggGoalHook,
    ll::memory::HookPriority::Highest,
    ::LayEggGoal,
    &::LayEggGoal::$isValidTarget,
    bool,
    ::BlockSource&    region,
    ::BlockPos const& pos
) {
    // 获取领地注册表实例
    auto* db   = PLand::getInstance().getLandRegistry();
    auto  land = db->getLandAt(pos, region.getDimensionId());

    // 如果在领地内且不允许实体破坏，则阻止产蛋
    if (land && !land->getPermTable().allowActorDestroy) {
        return false;
    }
    return origin(region, pos);
}

void registeronLayEggGoalHook() { onLayEggGoalHook::hook(); }
void unregisteronLayEggGoalHook() { onLayEggGoalHook::unhook(); }

void registerMobHurtHook() { MobHurtHook::hook(); }
void unregisterMobHurtHook() { MobHurtHook::unhook(); }

void registerOnFishingHookHitHook() { onFishingHookHitHook::hook(); }
void unregisterOnFishingHookHitHook() { onFishingHookHitHook::unhook(); }
} // namespace land
