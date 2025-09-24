#pragma once
#include "ISelector.h"
#include "ll/api/coro/InterruptableSleep.h"
#include "ll/api/event/ListenerBase.h"
#include "pland/Global.h"
#include <unordered_map>


namespace land {


/**
 * @brief 选区管理器
 * @note 由 PLand 管理 (RAII)
 */
class SelectorManager final {
    std::unordered_map<mce::UUID, std::unique_ptr<ISelector>> mSelectors{};
    std::unordered_map<mce::UUID, std::time_t>                mStabilization{};
    ll::event::ListenerPtr                                    mListener{nullptr};
    std::shared_ptr<std::atomic<bool>>                        mCoroStop{nullptr};
    std::shared_ptr<ll::coro::InterruptableSleep>             mInterruptableSleep{nullptr};

    LDAPI bool startSelectionImpl(std::unique_ptr<ISelector> selector);

public:
    LD_DISABLE_COPY_AND_MOVE(SelectorManager);

    explicit SelectorManager();
    ~SelectorManager();

    // 玩家是否正在选区 & 是否有选区任务
    LDNDAPI bool hasSelector(mce::UUID const& uuid) const;
    LDNDAPI bool hasSelector(Player& player) const;

    // 获取选区任务
    LDNDAPI ISelector* getSelector(mce::UUID const& uuid) const;
    LDNDAPI ISelector* getSelector(Player& player) const;

    // 开始选区
    template <typename T>
        requires std::is_base_of_v<ISelector, T> && std::is_final_v<T>
    bool startSelection(std::unique_ptr<T> selector) {
        return startSelectionImpl(std::move(selector));
    }

    // 停止选区
    LDAPI void stopSelection(mce::UUID const& uuid);
    LDAPI void stopSelection(Player& player);

    using ForEachFunc = std::function<bool(mce::UUID const&, ISelector*)>;
    LDAPI void forEach(ForEachFunc const& func) const;
};


} // namespace land