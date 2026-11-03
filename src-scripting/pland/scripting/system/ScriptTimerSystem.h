#pragma once
#include "pland/scripting/pch.h"

#include <ll/api/Expected.h>
#include <ll/api/coro/ConditionVariable.h>
#include <ll/api/coro/CoroPromise.h>


namespace land::scripting {

class ScriptTimerSystem {
public:
    using TaskId = uint32_t;
    inline static constexpr TaskId kInvalidTaskId{0};

    explicit ScriptTimerSystem();
    ~ScriptTimerSystem();

    using Finalizer = std::function<void(TaskId id)>;

    TaskId newTimeout(jspp::Local<jspp::Function> callback, int64_t timeout, Finalizer finalizer = nullptr);

    TaskId newInterval(jspp::Local<jspp::Function> callback, int64_t timeout, Finalizer finalizer = nullptr);

    bool cancelTask(TaskId taskId);

    bool hasTask(TaskId taskId) const;

    void clearAll();

private:
    struct CancelToken {
        ll::coro::ConditionVariable sleep{};
        bool                        abort{false};
    };
    using CancelTokenPtr = std::shared_ptr<CancelToken>;

    TaskId                                      mNextId{1};
    absl::flat_hash_map<TaskId, CancelTokenPtr> mCancelTokens{};
};

} // namespace land::scripting
