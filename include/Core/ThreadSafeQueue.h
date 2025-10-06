#ifndef CORE_PACKETQUEUE_
#define CORE_PACKETQUEUE_

#include <atomic>
#include <optional>
#include <semaphore>
#include <xenium/ramalhete_queue.hpp>
#include <xenium/reclamation/generic_epoch_based.hpp>

/**
 * @brief A generic, thread-safe queue for concurrent data access.
 * @details This class wraps a lockless ramalhete_queue and which provides much
 * faster access than mutex-based queue. It uses semaphore to provide a blocking
 * `WaitAndPop` method, which efficiently waits for an item to become available
 * without busy-waiting.
 * @tparam T The type of elements to be stored in the queue.
 */
template <typename T>
class ThreadSafeQueue {
 public:
  ThreadSafeQueue() : sem(0) {}
  ~ThreadSafeQueue() = default;
  ThreadSafeQueue(const ThreadSafeQueue&) = delete;
  ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

  void Push(T value) {
    queue.push(std::make_unique<T>(std::move(value)));
    sem.release();
    return;
  }

  // Blocking Pop
  std::optional<T> WaitAndPop() {
    sem.acquire();
    if (isDone) {
      sem.release();
      return std::nullopt;
    }
    std::unique_ptr<T> valueptr;
    while (!queue.try_pop(valueptr));
    return std::move(*valueptr);
  }

  // Non-Blocking Pop
  bool TryPop(T& value) {
    std::unique_ptr<T> valueptr;
    if (queue.try_pop(valueptr)) {
      value = std::move(*valueptr);
      return true;
    }
    return false;
  }

  void Shutdown() {
    isDone = true;
    sem.release();
  }

 private:
  xenium::ramalhete_queue<
      std::unique_ptr<T>,
      xenium::policy::reclaimer<xenium::reclamation::epoch_based<>>>
      queue;
  std::counting_semaphore<> sem;
  std::atomic<bool> isDone;
};

#endif /* CORE_PACKETQUEUE_ */
