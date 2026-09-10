#include "pland/internal/interceptor/EventInterceptor.h"
#include "pland/internal/interceptor/InterceptorConfig.h"
#include "pland/internal/interceptor/helper/EventTrace.h"
#include "pland/internal/interceptor/helper/InterceptorHelper.h"

#include "mc/server/ServerPlayer.h"
#include "mc/util/NamedMolangScript.h"
#include "mc/world/actor/ActorDefinition.h"
#include "mc/world/actor/ActorDefinitionGroup.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/actor/spawn_category/Type.h"
#include "mc/world/level/Level.h"


#include "ll/api/event/EventBus.h"
#include "ll/api/event/entity/ActorHurtEvent.h"
#include "ll/api/event/world/SpawnMobEvent.h"


namespace land::internal::interceptor {

void EventInterceptor::setupLLEntityListeners() {
    auto registry = &PLand::getInstance().getLandRegistry();
    auto bus      = &ll::event::EventBus::getInstance();

    // 依据实体定义的 vanilla 生成分组 (spawn rule 的 population_control => SpawnCategory),
    registerListenerIf<&InterceptorConfig::Listeners::SpawningMobEvent>([bus, registry]() {
        return bus->emplaceListener<ll::event::SpawningMobEvent>([registry](ll::event::SpawningMobEvent& ev) {
            TRACE_THIS_EVENT(ll::event::SpawningMobEvent);

            if (!ev.naturalSpawn()) {
                TRACE_LOG("not natural spawn");
                return; // 非自然生成 (刷怪笼/指令/组件骑手等) 不走 population 管线, 由 SpawnedMobEvent 兜底
            }

            auto* group = ev.blockSource().getLevel().getEntityDefinitions();
            if (!group) {
                TRACE_LOG("no entity definitions");
                return;
            }

            // getFullName() 携带事件后缀 ("minecraft:spider<>"), 定义表的键不含事件部分
            auto defName = std::string_view{ev.identifier().getFullName()};
            defName      = defName.substr(0, defName.find('<'));

            auto def = group->tryGetDefinition(std::string{defName});
            if (!def.mPtr) {
                TRACE_LOG("no definition for {}", defName);
                return;
            }

            auto category = def.mPtr->mDescription->mSpawnCategoryDescription->mSpawnCategory;
            TRACE_LOG("identifier={}, category={}", ev.identifier().getFullName(), magic_enum::enum_name(category));

            auto land = registry->getLandAt(ev.pos(), ev.blockSource().getDimensionId());
            if (!land) {
                TRACE_LOG("no land at {}", ev.pos());
                return; // 领地外 => 放行
            }

            using SpawnCategory::Type;
            switch (category) {
            case Type::Monster:
                if (!hasEnvironmentPermission<&EnvironmentPerms::allowMonsterSpawn>(land)) {
                    ev.cancel(); // 生成前取消, 不产生实体
                }
                break;
            case Type::Creature:
            case Type::WaterCreature:
            case Type::Axolotls:
            case Type::UndergroundWaterCreature:
            case Type::WaterAmbient:
                if (!hasEnvironmentPermission<&EnvironmentPerms::allowAnimalSpawn>(land)) {
                    ev.cancel();
                }
                break;
            default: // Ambient / Misc: 现有权限模型未覆盖, 放行
                TRACE_LOG(
                    "category not covered: {}, mob={}",
                    magic_enum::enum_name(category),
                    ev.identifier().getFullName()
                );
                break;
            }
        });
    });

    // 兜底层: 非自然生成 (刷怪笼/组件骑手等) 的后置拦截
    registerListenerIf<&InterceptorConfig::Listeners::SpawnedMobEvent>([bus, registry]() {
        return bus->emplaceListener<ll::event::SpawnedMobEvent>([registry](ll::event::SpawnedMobEvent& ev) {
            TRACE_THIS_EVENT(ll::event::SpawnedMobEvent);

            if (ev.naturalSpawn()) {
                return; // 自然生成由 SpawningMobEvent 层负责
            }

            auto mob = ev.mob();
            if (!mob) {
                return; // 因为事件是拦截特定类型实体生成，如果实体不存在这里直接跳过
            }

            auto     spawner = ev.spawner();
            BlockPos pos     = ev.pos();

            TRACE_LOG(
                "spawner={}, mob={}",
                spawner ? spawner->getTypeName() : "null",
                mob ? mob->getTypeName() : "null"
            );

            auto land = registry->getLandAt(pos, mob->getDimensionId());

            bool allowMonster = hasEnvironmentPermission<&EnvironmentPerms::allowMonsterSpawn>(land);
            bool allowAnimal  = hasEnvironmentPermission<&EnvironmentPerms::allowAnimalSpawn>(land);

            bool isMonster = mob->hasCategory(::ActorCategory::Monster) || mob->hasFamily("monster");
            if ((isMonster && !allowMonster) || (!isMonster && !allowAnimal)) {
                mob->despawn();
            }
        });
    });

    registerListenerIf<&InterceptorConfig::Listeners::ActorHurtEvent>([bus]() {
        return bus->emplaceListener<ll::event::ActorHurtEvent>([](ll::event::ActorHurtEvent& ev) {
            TRACE_THIS_EVENT(ll::event::ActorHurtEvent);

            auto& actor  = ev.self();
            auto& source = ev.source();

            TRACE_LOG(
                "actor={}, source={}, cause={}",
                actor.getTypeName(),
                magic_enum::enum_name(source.getEntityType()),
                magic_enum::enum_name(source.getCause())
            );

            if (source.getEntityType() != ActorType::Player) {
                TRACE_LOG("source is not player");

                // Fix [#245](https://github.com/IceBlcokMC/PLand/issues/245)
                // 焰火火箭爆炸的范围伤害, 其 ActorDamageByActorSource 将"攻击者"归属为
                // 火箭实体本身 (cause=Fireworks) 而非发射玩家, 归因回发射者后逐受害者拦截
                if (source.getCause() == ::SharedTypes::Legacy::ActorDamageCause::Fireworks) {
                    TRACE_LOG("source is firework rocket, checking source owner");
                    auto* rocket = actor.getLevel().fetchEntity(source.getEntityUniqueID(), false);
                    if (rocket) {
                        TRACE_LOG("rocket found, checking rocket owner, rocket={}", rocket->getTypeName());
                        // TODO: 这里获取不到 Owner
                        auto* owner = rocket->getOwner();
                        if (owner && owner->getEntityTypeId() == ActorType::Player) {
                            auto& shooter = static_cast<Player&>(*owner);
                            TRACE_LOG(
                                "rocket owner is player, checking player damage permission, player={}",
                                shooter.getRealName()
                            );
                            if (!hasPlayerDamagePermission(actor, shooter.getUuid())) {
                                ev.cancel();
                            }
                        }
                    }
                }
                return;
            }

            auto player = actor.getLevel().getPlayer(source.getEntityUniqueID());
            if (!player) {
                TRACE_LOG("source player not found");
                return;
            }

            auto& uuid = player->getUuid();
            if (!hasPlayerDamagePermission(actor, uuid)) {
                ev.cancel();
            }
        });
    });
}

} // namespace land::internal::interceptor
