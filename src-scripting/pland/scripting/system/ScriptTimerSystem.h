#pragma once
#include "pland/scripting/pch.h"

#include <ll/api/coro/CoroPromise.h>
#include <ll/api/coro/InterruptableSleep.h>

namespace land::scripting {

class ScriptTimerSystem {
public:
    using TaskId = uint32_t;
    inline static constexpr TaskId kInvalidTaskId{0};

    explicit ScriptTimerSystem();
    ~ScriptTimerSystem();

    TaskId newTimeout(jspp::Local<jspp::Function> callback, int64_t timeout);

    TaskId newInterval(jspp::Local<jspp::Function> callback, int64_t timeout);

    bool cancelTask(TaskId taskId);

    bool hasTask(TaskId taskId) const;

    void clearAll();

private:
    struct CancelToken {
        ll::coro::InterruptableSleep sleep{};
        bool                         abort{false};
    };
    using CancelTokenPtr = std::shared_ptr<CancelToken>;

    TaskId                                      mNextId{1};
    absl::flat_hash_map<TaskId, CancelTokenPtr> mCancelTokens{};
};

} // namespace land::scripting
