#pragma once
#include "pland/infra/HashedStringView.h"

namespace land::HashedTypeName {

constexpr HashedStringView Minecart   = {"minecraft:minecart"};
constexpr HashedStringView IsMinecart = {"minecraft:is_minecart"}; // Item Tag

constexpr HashedStringView Boat  = {"minecraft:boat"};
constexpr HashedStringView Boats = {"minecraft:boats"}; // Item Tag

constexpr HashedStringView ChestBoat = {"minecraft:chest_boat"};

constexpr HashedStringView FishingHook = {"minecraft:fishing_hook"};

constexpr HashedStringView DragonEgg = {"minecraft:dragon_egg"};

constexpr HashedStringView Player = {"minecraft:player"};

constexpr HashedStringView Trident = {"minecraft:trident"};

} // namespace land::HashedTypeName