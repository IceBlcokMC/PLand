#include "LandDimensionChunkMap.h"
#include "BidirectionalMap.h"
#include "ChunkEncoder.h"

#include "pland/land/Land.h"

namespace land ::internal {

LandDimensionChunkMap::LandDimensionChunkMap() = default;

bool LandDimensionChunkMap::hasDimension(LandDimid dimid) const { return mMap.contains(dimid); }

bool LandDimensionChunkMap::hasChunk(LandDimid dimid, ChunkID chunkid) const {
    if (!mMap.contains(dimid)) {
        return false;
    }
    return mMap.at(dimid).has_key(chunkid);
}

bool LandDimensionChunkMap::hasLand(LandDimid dimid, LandID landid) const {
    if (!mMap.contains(dimid)) {
        return false;
    }
    return mMap.at(dimid).has_value(landid);
}

LandDimensionChunkMap::LandSet const* LandDimensionChunkMap::queryLand(LandDimid dimId, ChunkID chunkId) const {
    auto iter = mMap.find(dimId);
    if (iter == mMap.end()) {
        return nullptr;
    }
    auto& chunkMap = iter->second.forward_map();
    auto  iter2    = chunkMap.find(chunkId);
    if (iter2 == chunkMap.end()) {
        return nullptr;
    }
    return &iter2->second;
}

LandDimensionChunkMap::ChunkSet const* LandDimensionChunkMap::queryChunk(LandDimid dimId, LandID landId) const {
    auto iter = mMap.find(dimId);
    if (iter == mMap.end()) {
        return nullptr;
    }
    auto& landMap = iter->second.reverse_map();
    auto  iter2   = landMap.find(landId);
    if (iter2 == landMap.end()) {
        return nullptr;
    }
    return &iter2->second;
}

void LandDimensionChunkMap::forEachLandId(LandDimid dimId, std::function<bool(LandID)> const& visitor) const {
    auto dimIter = mMap.find(dimId);
    if (dimIter == mMap.end()) {
        return;
    }

    // 反向表键集合即该维度已登记的领地
    for (auto const& entry : dimIter->second.reverse_map()) {
        if (!visitor(entry.first)) {
            return;
        }
    }
}

void LandDimensionChunkMap::addLand(std::shared_ptr<Land> const& land) {
    auto landDimId = land->getDimensionId();
    auto landId    = land->getId();

    auto chunkIds =
        land->getAABB().getChunks() | std::views::transform([](auto& c) { return ChunkEncoder::encode(c.x, c.z); });

    auto& dim = mMap[landDimId];
    for (auto chunkId : chunkIds) {
        dim.insert(chunkId, landId);
    }
}

void LandDimensionChunkMap::removeLand(std::shared_ptr<Land> const& land) {
    auto landDimId = land->getDimensionId();
    auto landId    = land->getId();

    if (!mMap.contains(landDimId)) return;

    auto& dim         = mMap.at(landDimId);
    auto  chunkSetPtr = queryChunk(landDimId, landId);
    if (!chunkSetPtr) return;

    auto chunkSet = *chunkSetPtr;
    for (auto chunkId : chunkSet) {
        dim.erase_value(chunkId, landId);
    }
}

void LandDimensionChunkMap::refreshRange(std::shared_ptr<Land> const& land) {
    auto landDimId = land->getDimensionId();
    if (!mMap.contains(landDimId)) {
        return;
    }
    removeLand(land);
    addLand(land);
}


} // namespace land::internal