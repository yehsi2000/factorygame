#ifndef CORE_PACKETQUEUE_
#define CORE_PACKETQUEUE_

#include <condition_variable>
#include <mutex>
#include <queue>

/**
 * @brief A generic, thread-safe queue for concurrent data access.
 * @details This class wraps a standard std::queue and protects it with a mutex
 * to allow safe pushing and popping of elements from multiple threads. It uses
 * a condition variable to provide a blocking `WaitAndPop` method, which
 * efficiently waits for an item to become available without busy-waiting.
 * @tparam T The type of elements to be stored in the queue.
 */
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
