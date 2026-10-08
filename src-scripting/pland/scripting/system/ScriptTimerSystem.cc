#include "ScriptTimerSystem.h"

#include "pland/PLand.h"

#include <ll/api/coro/CoroTask.h>
#include <ll/api/thread/ServerThreadExecutor.h>

namespace land::scripting {

ScriptTimerSystem::ScriptTimerSystem() = default;
ScriptTimerSystem::~ScriptTimerSystem() { clearAll(); }

ScriptTimerSystem::TaskId ScriptTimerSystem::newTimeout(jspp::Local<jspp::Function> callback, int64_t timeout) {
    auto id    = mNextId++;
    auto token = std::make_shared<CancelToken>();

    auto tracked = jspp::TrackedGlobal<jspp::Function>::create(std::move(callback));
    ll::coro::keepThis([tracked, timeout, token]() -> ll::coro::CoroTask<> {
        co_await token->sleep.sleepFor(std::chrono::milliseconds{timeout});
        if (token->abort) {
            co_return;
        }

        auto& persint = tracked->global();
        auto  _       = jspp::EngineScope{persint.engine()};
        try {
            auto fn = persint.get();
            (void)fn.call({});
        } catch (jspp::Exception const& e) {
            PLand::getInstance().getSelf().getLogger().error("Uncaught Exception: {}\n{}", e.what(), e.stacktrace());
        }
        co_return;
    }).launch(ll::thread::ServerThreadExecutor::getDefault(), [this, id](auto) { this->mCancelTokens.erase(id); });
    if (!mCancelTokens.emplace(id, std::move(token)).second) {
        token->abort = true;
        token->sleep.interrupt(true);
        return kInvalidTaskId;
    }
    return id;
}

ScriptTimerSystem::TaskId ScriptTimerSystem::newInterval(jspp::Local<jspp::Function> callback, int64_t timeout) {
    auto id    = mNextId++;
    auto token = std::make_shared<CancelToken>();

    auto tracked = jspp::TrackedGlobal<jspp::Function>::create(std::move(callback));
    ll::coro::keepThis([tracked, timeout, token]() -> ll::coro::CoroTask<> {
        while (!token->abort) {
            co_await token->sleep.sleepFor(std::chrono::milliseconds{timeout});
            if (token->abort) {
                co_return;
            }

            auto& persint = tracked->global();
            auto  _       = jspp::EngineScope{persint.engine()};
            try {
                auto fn = persint.get();
                (void)fn.call({});
            } catch (jspp::Exception const& e) {
                PLand::getInstance().getSelf().getLogger().error(
                    "Uncaught Exception: {}\n{}",
                    e.what(),
                    e.stacktrace()
                );
            }
        }
        co_return;
    }).launch(ll::thread::ServerThreadExecutor::getDefault(), [this, id](auto) { this->mCancelTokens.erase(id); });
    if (!mCancelTokens.emplace(id, std::move(token)).second) {
        token->abort = true;
        token->sleep.interrupt(true);
        return kInvalidTaskId;
    }
    return id;
}

bool ScriptTimerSystem::cancelTask(TaskId taskId) {
    auto iter = mCancelTokens.find(taskId);
    if (iter == mCancelTokens.end()) {
        return false;
    }
    iter->second->abort = true;
    iter->second->sleep.interrupt(true);
    mCancelTokens.erase(iter);
    return true;
}

bool ScriptTimerSystem::hasTask(TaskId taskId) const { return mCancelTokens.contains(taskId); }

void ScriptTimerSystem::clearAll() {
    auto copy = mCancelTokens; // 拷贝一份，避免协程清理函数导致迭代器失效
    for (auto& token : copy | std::views::values) {
        token->abort = true;
        token->sleep.interrupt(true);
    }
}


} // namespace land::scripting