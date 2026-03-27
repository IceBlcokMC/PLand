#pragma once
#include <mutex>
#include <queue>

namespace land {

template <typename T>
class TimerScheduler {
public:
    struct Task {
        time_t timestamp;
        T      data;

        bool operator>(Task const& other) const { return timestamp > other.timestamp; }
    };

private:
    using Queue = std::priority_queue<Task, std::vector<Task>, std::greater<Task>>;
    Queue      mQueue;
    std::mutex mMutex;

public:
    void push(time_t timestamp, T data) {
        std::lock_guard lock(mMutex);
        mQueue.push({timestamp, std::move(data)});
    }

    std::vector<T> popDueTasks(time_t now) {
        std::vector<T>  dueTasks;
        std::lock_guard lock(mMutex);
        while (!mQueue.empty() && mQueue.top().timestamp <= now) {
            dueTasks.push_back(std::move(mQueue.top().data));
            mQueue.pop();
        }
        return dueTasks;
    }

    void clear() {
        std::lock_guard lock(mMutex);
        mQueue = decltype(mQueue){};
    }
};

} // namespace land
