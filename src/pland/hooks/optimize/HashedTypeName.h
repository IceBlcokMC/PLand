#pragma once
#include "pland/infra/HashedStringView.h"

#include <mc/world/item/ItemTag.h>

namespace land::HashedTypeName {

constexpr HashedStringView  Minecart = {"minecraft:minecart"};
inline static ItemTag const MinecartTag{"minecraft:is_minecart"}; // Item Tag

constexpr HashedStringView  Boat = {"minecraft:boat"};
inline static ItemTag const BoatsTag{"minecraft:boats"}; // Item Tag
inline static ItemTag const BoatTag{Boat.mStr.data()};

constexpr HashedStringView ChestBoat = {"minecraft:chest_boat"};

constexpr HashedStringView FishingHook = {"minecraft:fishing_hook"};

constexpr HashedStringView DragonEgg = {"minecraft:dragon_egg"};

constexpr HashedStringView Player = {"minecraft:player"};

constexpr HashedStringView Trident = {"minecraft:trident"};

} // namespace land::HashedTypeName