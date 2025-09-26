
#include "pland/hooks/EventListener.h"
#include "pland/hooks/listeners/ListenerHelper.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerAttackEvent.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"
#include "ll/api/event/player/PlayerInteractBlockEvent.h"
#include "ll/api/event/player/PlayerPickUpItemEvent.h"
#include "ll/api/event/player/PlayerPlaceBlockEvent.h"
#include "ll/api/event/player/PlayerUseItemEvent.h"

#include "mc/deps/core/string/HashedString.h"
#include "mc/world/item/BucketItem.h"
#include "mc/world/item/FishingRodItem.h"
#include "mc/world/item/HatchetItem.h"
#include "mc/world/item/HoeItem.h"
#include "mc/world/item/HorseArmorItem.h"
#include "mc/world/item/Item.h"
#include "mc/world/item/ItemTag.h"
#include "mc/world/item/ShovelItem.h"
#include "mc/world/level/block/BlastFurnaceBlock.h"
#include "mc/world/level/block/FurnaceBlock.h"
#include "mc/world/level/block/HangingSignBlock.h"
#include "mc/world/level/block/ShulkerBoxBlock.h"
#include "mc/world/level/block/SignBlock.h"
#include "mc/world/level/block/SmokerBlock.h"

#include "pland/PLand.h"
#include "pland/infra/Config.h"
#include "pland/land/LandRegistry.h"
#include "pland/utils/McUtils.h"

#include <string_view>
#include <unordered_map>


namespace land {

// These maps are used by PlayerInteractBlockEvent, so they stay in this file.
static std::unordered_map<std::string_view, bool LandPermTable::*> ItemSpecificPermissionMap;
static std::unordered_map<std::string_view, bool LandPermTable::*> BlockSpecificPermissionMap;
static std::unordered_map<std::string_view, bool LandPermTable::*> BlockFunctionalPermissionMap;

// A map to convert permission names (from config) to member pointers.
static const std::unordered_map<std::string, bool LandPermTable::*> StringToPermPtrMap = {
    {          "allowPlace",           &LandPermTable::allowPlace},
    {    "useFlintAndSteel",     &LandPermTable::useFlintAndSteel},
    {         "useBoneMeal",          &LandPermTable::useBoneMeal},
    {"allowAttackDragonEgg", &LandPermTable::allowAttackDragonEgg},
    {              "useBed",               &LandPermTable::useBed},
    {      "allowOpenChest",       &LandPermTable::allowOpenChest},
    {         "useCampfire",          &LandPermTable::useCampfire},
    {        "useComposter",         &LandPermTable::useComposter},
    {        "useNoteBlock",         &LandPermTable::useNoteBlock},
    {          "useJukebox",           &LandPermTable::useJukebox},
    {             "useBell",              &LandPermTable::useBell},
    { "useDaylightDetector",  &LandPermTable::useDaylightDetector},
    {          "useLectern",           &LandPermTable::useLectern},
    {         "useCauldron",          &LandPermTable::useCauldron},
    {    "useRespawnAnchor",     &LandPermTable::useRespawnAnchor},
    {       "editFlowerPot",        &LandPermTable::editFlowerPot},
    {        "allowDestroy",         &LandPermTable::allowDestroy},
    { "useCartographyTable",  &LandPermTable::useCartographyTable},
    {    "useSmithingTable",     &LandPermTable::useSmithingTable},
    {     "useBrewingStand",      &LandPermTable::useBrewingStand},
    {            "useAnvil",             &LandPermTable::useAnvil},
    {       "useGrindstone",        &LandPermTable::useGrindstone},
    {  "useEnchantingTable",   &LandPermTable::useEnchantingTable},
    {           "useBarrel",            &LandPermTable::useBarrel},
    {           "useBeacon",            &LandPermTable::useBeacon},
    {           "useHopper",            &LandPermTable::useHopper},
    {          "useDropper",           &LandPermTable::useDropper},
    {        "useDispenser",         &LandPermTable::useDispenser},
    {             "useLoom",              &LandPermTable::useLoom},
    {      "useStonecutter",       &LandPermTable::useStonecutter},
    {          "useCrafter",           &LandPermTable::useCrafter},
    {"useChiseledBookshelf", &LandPermTable::useChiseledBookshelf},
    {             "useCake",              &LandPermTable::useCake},
    {       "useComparator",        &LandPermTable::useComparator},
    {         "useRepeater",          &LandPermTable::useRepeater},
    {          "useBeeNest",           &LandPermTable::useBeeNest},
    {            "useVault",             &LandPermTable::useVault}
};

// Helper to load permissions from config
void loadPermissionMapsFromConfig() {
    auto logger = &land::PLand::getInstance().getSelf().getLogger();

    ItemSpecificPermissionMap.clear();
    BlockSpecificPermissionMap.clear();
    BlockFunctionalPermissionMap.clear();

    auto populateMap = [&](const auto& configMap, auto& targetMap, const std::string& mapName) {
        for (const auto& [itemName, permName] : configMap) {
            auto it = StringToPermPtrMap.find(permName);
            if (it != StringToPermPtrMap.end()) {
                targetMap[itemName] = it->second;
            } else {
                logger->warn(
                    "Permission '{}' for item '{}' in '{}' map not found. Ignoring.",
                    permName,
                    itemName,
                    mapName
                );
            }
        }
    };

    populateMap(Config::cfg.protection.permissionMaps.itemSpecific, ItemSpecificPermissionMap, "itemSpecific");
    populateMap(Config::cfg.protection.permissionMaps.blockSpecific, BlockSpecificPermissionMap, "blockSpecific");
    populateMap(Config::cfg.protection.permissionMaps.blockFunctional, BlockFunctionalPermissionMap, "blockFunctional");
}


void EventListener::registerLLPlayerListeners() {
    loadPermissionMapsFromConfig();
    auto* db     = PLand::getInstance().getLandRegistry();
    auto* bus    = &ll::event::EventBus::getInstance();
    auto* logger = &land::PLand::getInstance().getSelf().getLogger();

    RegisterListenerIf(Config::cfg.listeners.PlayerDestroyBlockEvent, [&]() {
        return bus->emplaceListener<ll::event::PlayerDestroyBlockEvent>(
            [db, logger](ll::event::PlayerDestroyBlockEvent& ev) {
                auto& player   = ev.self();
                auto& blockPos = ev.pos();

                EVENT_TRACE(
                    "PlayerDestroyBlockEvent",
                    EVENT_TRACE_LOG,
                    "player={}, pos={}",
                    player.getRealName(),
                    blockPos.toString()
                );

                auto land = db->getLandAt(blockPos, player.getDimensionId());
                if (PreCheckLandExistsAndPermission(land, player.getUuid())) {
                    EVENT_TRACE("PlayerDestroyBlockEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                    return;
                }

                auto& tab = land->getPermTable();
                if (tab.allowDestroy) {
                    EVENT_TRACE("PlayerDestroyBlockEvent", EVENT_TRACE_PASS, "allowDestroy allowed");
                    return;
                }

                ev.cancel();
                EVENT_TRACE("PlayerDestroyBlockEvent", EVENT_TRACE_CANCEL, "allowDestroy denied");
            }
        );
    });

    RegisterListenerIf(Config::cfg.listeners.PlayerPlacingBlockEvent, [&]() {
        return bus->emplaceListener<ll::event::PlayerPlacingBlockEvent>(
            [db, logger](ll::event::PlayerPlacingBlockEvent& ev) {
                auto& player   = ev.self();
                auto  blockPos = mc_utils::face2Pos(ev.pos(), ev.face());

                EVENT_TRACE(
                    "PlayerPlacingBlockEvent",
                    EVENT_TRACE_LOG,
                    "player={}, pos={}",
                    player.getRealName(),
                    blockPos.toString()
                );

                auto land = db->getLandAt(blockPos, player.getDimensionId());
                if (PreCheckLandExistsAndPermission(land, player.getUuid())) {
                    EVENT_TRACE("PlayerPlacingBlockEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                    return;
                }

                auto& tab = land->getPermTable();
                if (tab.allowPlace) {
                    EVENT_TRACE("PlayerPlacingBlockEvent", EVENT_TRACE_PASS, "allowPlace allowed");
                    return;
                }

                ev.cancel();
                EVENT_TRACE("PlayerPlacingBlockEvent", EVENT_TRACE_CANCEL, "allowPlace denied");
            }
        );
    });

    RegisterListenerIf(Config::cfg.listeners.PlayerInteractBlockEvent, [&]() {
        return bus->emplaceListener<ll::event::PlayerInteractBlockEvent>(
            [db, logger](ll::event::PlayerInteractBlockEvent& ev) {
                auto&      player       = ev.self();
                auto&      pos          = ev.blockPos();
                auto&      itemStack    = ev.item();
                auto const itemTypeName = itemStack.getTypeName();

                EVENT_TRACE(
                    "PlayerInteractBlockEvent",
                    EVENT_TRACE_LOG,
                    "player={}, pos={}, item={}",
                    player.getRealName(),
                    pos.toString(),
                    itemTypeName
                );

                auto land = db->getLandAt(pos, player.getDimensionId());
                if (PreCheckLandExistsAndPermission(land, player.getUuid())) {
                    EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                    return;
                }

                auto const& tab = land->getPermTable();

                if (auto item = itemStack.getItem()) {
                    void** vftable = *reinterpret_cast<void** const*>(item);
                    if (vftable == BucketItem::$vftable()) {
                        CANCEL_AND_RETURN_IF(
                            !tab.useBucket,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "useBucket denied")
                        );
                    } else if (vftable == HatchetItem::$vftable()) {
                        CANCEL_AND_RETURN_IF(
                            !tab.allowAxePeeled,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "allowAxePeeled denied")
                        );
                    } else if (vftable == HoeItem::$vftable()) {
                        CANCEL_AND_RETURN_IF(
                            !tab.useHoe,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "useHoe denied")
                        );
                    } else if (vftable == ShovelItem::$vftable()) {
                        CANCEL_AND_RETURN_IF(
                            !tab.useShovel,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "useShovel denied")
                        );
                    } else if (item->hasTag(HashedString("minecraft:boat"))
                               || item->hasTag(HashedString("minecraft:boats"))) {
                        CANCEL_AND_RETURN_IF(
                            !tab.placeBoat,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "placeBoat denied")
                        );
                    } else if (item->hasTag(HashedString("minecraft:is_minecart"))) {
                        CANCEL_AND_RETURN_IF(
                            !tab.placeMinecart,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "placeMinecart denied")
                        );
                    }
                }
                if (auto it = ItemSpecificPermissionMap.find(itemTypeName);
                    it != ItemSpecificPermissionMap.end() && !(tab.*(it->second))) {
                    EVENT_TRACE(
                        "PlayerInteractBlockEvent",
                        EVENT_TRACE_CANCEL,
                        "Item check: '{}', specific permission is false.",
                        itemTypeName
                    );
                    ev.cancel();
                    return;
                }


                if (auto block = ev.block()) {
                    auto const& typeName    = block->getTypeName();
                    auto const& legacyBlock = block->getBlockType();

                    if (auto iter = BlockSpecificPermissionMap.find(typeName);
                        iter != BlockSpecificPermissionMap.end() && !(tab.*(iter->second))) {
                        EVENT_TRACE(
                            "PlayerInteractBlockEvent",
                            EVENT_TRACE_CANCEL,
                            "Block check: '{}', specific permission is false.",
                            typeName
                        );
                        ev.cancel();
                        return;
                    }

                    if (auto iter = BlockFunctionalPermissionMap.find(typeName);
                        iter != BlockFunctionalPermissionMap.end() && !(tab.*(iter->second))) {
                        EVENT_TRACE(
                            "PlayerInteractBlockEvent",
                            EVENT_TRACE_CANCEL,
                            "Block check: '{}', functional permission is false.",
                            typeName
                        );
                        ev.cancel();
                        return;
                    }

                    void** vftable = *reinterpret_cast<void** const*>(&legacyBlock);
                    if (legacyBlock.isButtonBlock()) {
                        CANCEL_AND_RETURN_IF(
                            !tab.useButton,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "useButton denied")
                        );
                    } else if (legacyBlock.isDoorBlock()) {
                        CANCEL_AND_RETURN_IF(
                            !tab.useDoor,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "useDoor denied")
                        );
                    } else if (legacyBlock.isFenceGateBlock()) {
                        CANCEL_AND_RETURN_IF(
                            !tab.useFenceGate,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "useFenceGate denied")
                        );
                    } else if (legacyBlock.isFenceBlock()) {
                        CANCEL_AND_RETURN_IF(
                            !tab.allowInteractEntity,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "allowInteractEntity denied")
                        );
                    } else if (legacyBlock.mIsTrapdoor) {
                        CANCEL_AND_RETURN_IF(
                            !tab.useTrapdoor,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "useTrapdoor denied")
                        );
                    } else if (vftable == SignBlock::$vftable() || vftable == HangingSignBlock::$vftable()) {
                        CANCEL_AND_RETURN_IF(
                            !tab.editSign,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "editSign denied")
                        );
                    } else if (vftable == ShulkerBoxBlock::$vftable()) {
                        CANCEL_AND_RETURN_IF(
                            !tab.useShulkerBox,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "useShulkerBox denied")
                        );
                    } else if (legacyBlock.isCraftingBlock()) {
                        CANCEL_AND_RETURN_IF(
                            !tab.useCraftingTable,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "useCraftingTable denied")
                        );
                    } else if (legacyBlock.isLeverBlock()) {
                        CANCEL_AND_RETURN_IF(
                            !tab.useLever,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "useLever denied")
                        );
                    } else if (vftable == BlastFurnaceBlock::$vftable()) {
                        CANCEL_AND_RETURN_IF(
                            !tab.useBlastFurnace,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "useBlastFurnace denied")
                        );
                    } else if (vftable == FurnaceBlock::$vftable()) {
                        CANCEL_AND_RETURN_IF(
                            !tab.useFurnace,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "useFurnace denied")
                        );
                    } else if (vftable == SmokerBlock::$vftable()) {
                        CANCEL_AND_RETURN_IF(
                            !tab.useSmoker,
                            EVENT_TRACE("PlayerInteractBlockEvent", EVENT_TRACE_CANCEL, "useSmoker denied")
                        );
                    }
                }
            }
        );
    });

    RegisterListenerIf(Config::cfg.listeners.PlayerAttackEvent, [&]() {
        return bus->emplaceListener<ll::event::PlayerAttackEvent>([db, logger](ll::event::PlayerAttackEvent& ev) {
            auto& player = ev.self();
            auto& mob    = ev.target();
            auto& pos    = mob.getPosition();

            EVENT_TRACE(
                "PlayerAttackEvent",
                EVENT_TRACE_LOG,
                "player={}, target={}, pos={}",
                player.getRealName(),
                mob.getTypeName(),
                pos.toString()
            );

            auto land = db->getLandAt(pos, player.getDimensionId());
            if (PreCheckLandExistsAndPermission(land, player.getUuid())) {
                EVENT_TRACE("PlayerAttackEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                return;
            }

            auto const& typeName = mob.getTypeName();
            auto const& tab      = land->getPermTable();

            if (Config::cfg.protection.mob.hostileMobTypeNames.contains(typeName)) {
                CANCEL_AND_RETURN_IF(
                    !tab.allowMonsterDamage,
                    EVENT_TRACE("PlayerAttackEvent", EVENT_TRACE_CANCEL, "allowMonsterDamage denied")
                );
            } else if (Config::cfg.protection.mob.specialMobTypeNames.contains(typeName)) {
                CANCEL_AND_RETURN_IF(
                    !tab.allowSpecialDamage,
                    EVENT_TRACE("PlayerAttackEvent", EVENT_TRACE_CANCEL, "allowSpecialDamage denied")
                );
            } else if (typeName == "minecraft:player") {
                CANCEL_AND_RETURN_IF(
                    !tab.allowPlayerDamage,
                    EVENT_TRACE("PlayerAttackEvent", EVENT_TRACE_CANCEL, "allowPlayerDamage denied")
                );
            } else if (Config::cfg.protection.mob.passiveMobTypeNames.contains(typeName)) {
                CANCEL_AND_RETURN_IF(
                    !tab.allowPassiveDamage,
                    EVENT_TRACE("PlayerAttackEvent", EVENT_TRACE_CANCEL, "allowPassiveDamage denied")
                );
            } else if (Config::cfg.protection.mob.customSpecialMobTypeNames.count(typeName)) {
                CANCEL_AND_RETURN_IF(
                    !tab.allowCustomSpecialDamage,
                    EVENT_TRACE("PlayerAttackEvent", EVENT_TRACE_CANCEL, "allowCustomSpecialDamage denied")
                );
            }
        });
    });

    RegisterListenerIf(Config::cfg.listeners.PlayerPickUpItemEvent, [&]() {
        return bus->emplaceListener<ll::event::PlayerPickUpItemEvent>([db,
                                                                       logger](ll::event::PlayerPickUpItemEvent& ev) {
            auto& player = ev.self();
            auto& item   = ev.itemActor();
            auto& pos    = item.getPosition();

            EVENT_TRACE(
                "PlayerPickUpItemEvent",
                EVENT_TRACE_LOG,
                "player={}, item={}, pos={}",
                player.getRealName(),
                item.getTypeName(),
                pos.toString()
            );

            auto land = db->getLandAt(pos, player.getDimensionId());
            if (PreCheckLandExistsAndPermission(land, player.getUuid())) {
                EVENT_TRACE("PlayerPickUpItemEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                return;
            }

            if (land->getPermTable().allowPickupItem) {
                EVENT_TRACE("PlayerPickUpItemEvent", EVENT_TRACE_PASS, "allowPickupItem allowed");
                return;
            }

            ev.cancel();
            EVENT_TRACE("PlayerPickUpItemEvent", EVENT_TRACE_CANCEL, "allowPickupItem denied");
        });
    });

    RegisterListenerIf(Config::cfg.listeners.PlayerUseItemEvent, [&]() {
        return bus->emplaceListener<ll::event::PlayerUseItemEvent>([db, logger](ll::event::PlayerUseItemEvent& ev) {
            auto& player = ev.self();
            auto& item   = ev.item();

            auto  typeName = item.getTypeName();
            auto& pos      = player.getPosition();

            EVENT_TRACE(
                "PlayerUseItemEvent",
                EVENT_TRACE_LOG,
                "player={}, item={}, pos={}",
                player.getRealName(),
                typeName,
                pos.toString()
            );

            auto land = db->getLandAt(pos, player.getDimensionId());
            if (PreCheckLandExistsAndPermission(land, player.getUuid())) {
                EVENT_TRACE("PlayerUseItemEvent", EVENT_TRACE_PASS, "land not found or permission allowed");
                return;
            }

            // patch https://github.com/engsr6982/PLand/issues/139
            CANCEL_AND_RETURN_IF(
                !land->getPermTable().allowProjectileCreate && typeName == "minecraft:trident",
                EVENT_TRACE("PlayerUseItemEvent", EVENT_TRACE_CANCEL, "allowProjectileCreate denied")
            );
        });
    });
}

} // namespace land
