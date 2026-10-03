#pragma once
#include "BidirectionalMap.h"
#include "ChunkEncoder.h"

#include "pland/Global.h"

#include "absl/container/flat_hash_map.h"

#include <functional>

namespace land {
class Land;
}

namespace land::internal {


/**
 * @brief 领地维度区块双向映射表
 *         / --> 区块 --> [领地]  # 查询领地
 * 维度 --|
 *        \ --> 领地 --> [区块]   # 查询区块
 */
class LandDimensionChunkMap {
public:
    using TypedBidirectionalMap = BidirectionalMap<ChunkID, LandID>; // 区块 --> 领地

    using DimensionMap = absl::flat_hash_map<LandDimid, TypedBidirectionalMap>;

    using ChunkSet = TypedBidirectionalMap::KeysSet;
    using LandSet  = TypedBidirectionalMap::ValuesSet;

public:
    LandDimensionChunkMap();

    /**
     * @brief 查询维度是否存在
     */
    [[nodiscard]] bool hasDimension(LandDimid dimId) const;

    /**
     * @brief 查询区块是否存在
     */
    [[nodiscard]] bool hasChunk(LandDimid dimId, ChunkID chunkId) const;

    /**
     * @brief 查询领地是否存在
     */
    [[nodiscard]] bool hasLand(LandDimid dimId, LandID landId) const;

    /**
     * @brief 查询某个区块下所有的领地
     */
    [[nodiscard]] LandSet const* queryLand(LandDimid dimId, ChunkID chunkId) const;

    /**
     * @brief 查询某个领地下所有的区块
     */
    [[nodiscard]] ChunkSet const* queryChunk(LandDimid dimId, LandID landId) const;

    /**
     * @brief 遍历某个维度下已登记的领地 ID
     * @param visitor 返回 false 结束遍历
     */
    void forEachLandId(LandDimid dimId, std::function<bool(LandID)> const& visitor) const;

    void addLand(std::shared_ptr<Land> const& land);

    void removeLand(std::shared_ptr<Land> const& land);

    void refreshRange(std::shared_ptr<Land> const& land);

private:
    DimensionMap mMap;
};

} // namespace land::internal
