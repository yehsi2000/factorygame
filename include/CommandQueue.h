#include <mutex>
#include <queue>
#include <functional>
#include <condition_variable>
#include <iostream>

class CommandQueue {
    std::queue<std::function<void()>> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void Push(std::function<void()> cmd) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(std::move(cmd));
        cv.notify_one();
    }

    std::function<void()> Pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&] {return !queue.empty();});
        auto cmd = std::move(queue.front());
        queue.pop();
        return cmd;
    }

    bool Empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }
};
