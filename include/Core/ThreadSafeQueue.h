#ifndef CORE_PACKETQUEUE_
#define CORE_PACKETQUEUE_

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class ThreadSafeQueue {
 public:
  void Push(T value) {
    std::lock_guard<std::mutex> lock(queueMutex);
    safeQueue.push(std::move(value));
    queueCV.notify_one();
  }

  // Blocking Pop
  T WaitAndPop() {
    std::unique_lock<std::mutex> lock(queueMutex);
    queueCV.wait(lock, [this] { return !safeQueue.empty(); });
    T value = std::move(safeQueue.front());
    safeQueue.pop();
    return value;
  }

  // Non-Blocking Pop
  bool TryPop(T& value) {
    std::lock_guard<std::mutex> lock(queueMutex);

    if (safeQueue.empty()) return false;

    value = std::move(safeQueue.front());
    safeQueue.pop();
    return true;
  }

 private:
  std::queue<T> safeQueue;
  std::mutex queueMutex;
  std::condition_variable queueCV;
};

#endif /* CORE_PACKETQUEUE_ */
