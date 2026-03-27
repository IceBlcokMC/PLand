#pragma once
#include "ll/api/coro/Executor.h"
#include "ll/api/thread/ThreadPoolExecutor.h"

#include <coroutine>

namespace land::coro_utils {

/**
 * @brief 协程跨线程切换器 (Coroutine Context Switcher)
 *
 * BDS (Bedrock Dedicated Server) 底层由多个线程共同驱动：
 * - 主逻辑线程 (MC_SERVER)：处理游戏刻(Tick)、实体更新、事件触发、发包。
 * - 后台线程池 (Streaming Pool)：处理区块加载、文件 IO、海量内存遍历等耗时操作。
 *
 * 转移机制原理:
 *
 * 1. 越狱拦截 (Bypass LeviLamina)：
 *    LeviLamina 的 `CoroTask<>` 的任务类型为 `struct CoroPromise<T> : CoroPromiseBase {}`
 *    而 `CoroPromiseBase` 中有一个模板方法 `decltype(auto) await_transform(T&& awaitable)`
 *    LeviLamina 框架默认会通过 `await_transform` 拦截 co_await 操作，试图把协程绑定在它启动时的那个线程里。
 *    但框架的拦截条件是：Awaiter 必须拥有 `setExecutor` 方法。
 *    我们不实现 `setExecutor`，从而成功越狱，让框架放行这段代码。
 *
 * 2. 剥夺与移交控制权 (Suspend & Transfer)：
 *    当代码执行到 `co_await SwitchExecutor{...}` 时：
 *    - `await_ready` 返回 false，强迫当前线程挂起（暂停执行）这个协程，并保存所有局部变量的状态。
 *    - 进入 `await_suspend`，拿到被打包好的协程句柄 (handle)，强行将其塞入目标线程池的执行队列
 *      `mTargetExecutor->execute(handle)`。
 *
 * 3. 异地复活 (Resume)：
 *    - 目标线程池（后台异步线程）从队列里取出这个 handle，执行它 (`handle.resume()`)。
 *    - 代码跨过了 `co_await`，成功在新线程中继续执行接下来的逻辑！
 *
 * 如何跳回启动时的线程？
 *    因为这个协程最初是 `launch(ServerThreadExecutor)` 在启动时的线程启动的，
 *    它的底层 Promise(CoroTask<T>) 对象始终存着启动时的线程的 Executor。
 *    当在后台线程调用 `co_await ll::coro::yield;` 时，框架 `YieldAwaiter await_transform(Yield const&)`
 *    返回启动时的线程 Executor， 并把协程句柄再塞回启动时的线程队列。这就形成了完美的闭环。
 */
struct SwitchExecutor {
    ll::coro::ExecutorRef mTargetExecutor;

    SwitchExecutor() = delete;

    /**
     * @param target 目标线程池执行器
     */
    explicit SwitchExecutor(ll::coro::ExecutorRef target) : mTargetExecutor(target) {
        assert(mTargetExecutor.has_value() && "Target executor cannot be null!");
    }

    // 强制要求挂起当前协程
    constexpr bool await_ready() const noexcept { return false; }

    // 协程挂起时触发，将当前协程上下文移交给目标线程池
    void await_suspend(std::coroutine_handle<> handle) const {
        assert(mTargetExecutor.has_value() && "Executor expired before suspension!");
        mTargetExecutor->execute(handle);
    }

    constexpr void await_resume() const noexcept {}
};


} // namespace land::coro_utils