#pragma once
#include "LandDimensionChunkMap.h"
#include "LandIdAllocator.h"
#include "StorageLayerError.h"
#include "ll/api/data/KeyValueDB.h"
#include "pland/Global.h"
#include "pland/land/Land.h"
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class Player;
class BlockPos;

namespace land {

struct PlayerSettings {
    bool        showEnterLandTitle{true};     // 是否显示进入领地提示
    bool        showBottomContinuedTip{true}; // 是否持续显示底部提示
    std::string localeCode{"server"};         // 语言 system / server / xxx

    LDNDAPI static std::string SYSTEM_LOCALE_CODE();
    LDNDAPI static std::string SERVER_LOCALE_CODE();
};

class LandTemplatePermTable;

class LandRegistry final {
    std::unique_ptr<ll::data::KeyValueDB>         mDB;                             // 领地数据库
    std::vector<mce::UUID>                        mLandOperators;                  // 领地操作员
    std::unordered_map<mce::UUID, PlayerSettings> mPlayerSettings;                 // 玩家设置
    std::unordered_map<LandID, SharedLand>        mLandCache;                      // 领地缓存
    mutable std::shared_mutex                     mMutex;                          // 读写锁
    std::unique_ptr<LandIdAllocator>              mLandIdAllocator{nullptr};       // 领地ID分配器
    LandDimensionChunkMap                         mDimensionChunkMap;              // 维度区块映射
    std::unique_ptr<LandTemplatePermTable>        mLandTemplatePermTable{nullptr}; // 领地模板权限表
    std::thread                                   mThread;                         // 线程
    std::atomic<bool>                             mThreadQuit{false};              // 线程退出标志
    mutable std::mutex                            mThreadMutex;                    // 线程互斥锁(仅 mThreadCV 使用)
    std::condition_variable                       mThreadCV;                       // 线程条件变量

    friend class DataConverter;

private: //! private 方法非线程安全
    void _loadOperators();
    void _loadPlayerSettings();
    void _loadLands();
    void _loadLandTemplatePermTable();

    void _openDatabaseAndEnsureVersion();
    void _migrateLegacyKeysIfNeeded(nlohmann::json& landData);

    void _buildDimensionChunkMap();

    LandID getNextLandID() const;

    Result<void, StorageLayerError::Error> _removeLand(SharedLand const& ptr);

    Result<void, StorageLayerError::Error> _addLand(SharedLand land);

public:
    LD_DISABLE_COPY_AND_MOVE(LandRegistry);
    explicit LandRegistry();
    ~LandRegistry();

    LDAPI void save();
    LDAPI bool save(Land const& land) const;

public:
    LDNDAPI bool isOperator(mce::UUID const& uuid) const;

    LDNDAPI bool addOperator(mce::UUID const& uuid);

    LDNDAPI bool removeOperator(mce::UUID const& uuid);

    LDNDAPI std::vector<mce::UUID> const& getOperators() const;

    LDNDAPI bool hasPlayerSettings(mce::UUID const& uuid) const;

    LDNDAPI PlayerSettings* getPlayerSettings(mce::UUID const& uuid);

    LDAPI bool setPlayerSettings(mce::UUID const& uuid, PlayerSettings settings);

    LDNDAPI LandTemplatePermTable& getLandTemplatePermTable() const;

    LDNDAPI bool hasLand(LandID id) const;

    LDAPI void refreshLandRange(SharedLand const& ptr); // 刷新领地范围

    LDNDAPI Result<void, StorageLayerError::Error> addOrdinaryLand(SharedLand const& land);

    LDNDAPI Result<void, StorageLayerError::Error> addSubLand(SharedLand const& parent, SharedLand const& sub);

    /**
     * @brief 移除领地
     * @deprecated 此接口已废弃，此接口实际调用 removeOrdinaryLand()，请使用 removeOrdinaryLand() 代替
     */
    [[deprecated("Please use removeOrdinaryLand() instead")]] LDAPI bool removeLand(LandID id);

    /**
     * @brief 移除普通领地
     */
    LDNDAPI Result<void, StorageLayerError::Error> removeOrdinaryLand(SharedLand const& ptr);

    /**
     * @brief 移除子领地
     */
    LDNDAPI Result<void, StorageLayerError::Error> removeSubLand(SharedLand const& ptr);

    /**
     * @brief 移除领地和其子领地
     */
    LDNDAPI Result<void, StorageLayerError::Error> removeLandAndSubLands(SharedLand const& ptr);

    /**
     * @brief 移除当前领地并提升子领地为普通领地
     */
    LDNDAPI Result<void, StorageLayerError::Error> removeLandAndPromoteSubLands(SharedLand const& ptr);

    /**
     * @brief 移除当前领地并移交子领地给当前领地的父领地
     */
    LDNDAPI Result<void, StorageLayerError::Error> removeLandAndTransferSubLands(SharedLand const& ptr);


public: // 领地查询API
    LDNDAPI WeakLand   getLandWeakPtr(LandID id) const;
    LDNDAPI SharedLand getLand(LandID id) const;
    LDNDAPI std::vector<SharedLand> getLands() const;
    LDNDAPI std::vector<SharedLand> getLands(std::vector<LandID> const& ids) const;
    LDNDAPI std::vector<SharedLand> getLands(LandDimid dimid) const;
    LDNDAPI std::vector<SharedLand> getLands(mce::UUID const& uuid, bool includeShared = false) const;
    LDNDAPI std::vector<SharedLand> getLands(mce::UUID const& uuid, LandDimid dimid) const;
    LDNDAPI std::unordered_map<mce::UUID, std::unordered_set<SharedLand>> getLandsByOwner() const;
    LDNDAPI std::unordered_map<mce::UUID, std::unordered_set<SharedLand>> getLandsByOwner(LandDimid dimid) const;

    LDNDAPI LandPermType getPermType(mce::UUID const& uuid, LandID id = 0, bool includeOperator = true) const;

    LDNDAPI SharedLand getLandAt(BlockPos const& pos, LandDimid dimid) const;

    LDNDAPI std::unordered_set<SharedLand> getLandAt(BlockPos const& center, int radius, LandDimid dimid) const;

    LDNDAPI std::unordered_set<SharedLand> getLandAt(BlockPos const& pos1, BlockPos const& pos2, LandDimid dimid) const;

    using FilterCallback = std::function<bool(SharedLand const&)>;
    LDNDAPI std::vector<SharedLand> getLandsWhere(FilterCallback const& callback) const;

    using ContextFilter = std::function<bool(LandContext const&)>;
    LDNDAPI std::vector<SharedLand> getLandsWhereRaw(ContextFilter const& filter) const;

public:
    LDAPI static ChunkID             EncodeChunkID(int x, int z);
    LDAPI static std::pair<int, int> DecodeChunkID(ChunkID id);

    static constexpr auto DbDirName              = "db";              // 数据库目录名
    static constexpr auto DbVersionKey           = "__version__";     // 数据库版本键
    static constexpr auto DbOperatorDataKey      = "operators";       // 操作员数据键
    static constexpr auto DbPlayerSettingDataKey = "player_settings"; // 玩家设置数据键
    static constexpr auto DbTemplatePermKey      = "template_perm";   // 领地模板权限表数据键
    static bool           isLandData(std::string_view key);           // 判断键是否为领地数据键
};


} // namespace land
