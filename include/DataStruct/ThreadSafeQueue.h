#pragma once

#include <atomic>
#include <optional>
#include <semaphore>
#include <xenium/ramalhete_queue.hpp>
#include <xenium/reclamation/hazard_pointer.hpp>

/**
 * @brief A generic, thread-safe queue for concurrent data access.
 * @details This class wraps a lockless ramalhete_queue and which provides much
 * faster access than mutex-based queue. It uses semaphore to provide a blocking
 * `WaitAndPop` method, which efficiently waits for an item to become available
 * without busy-waiting.
 * @tparam UniquePtr_T The unique_ptr of elements to be stored in the queue.
 */
template <typename UniquePtr_T>
class ThreadSafeQueue {
 public:
  ThreadSafeQueue() : sem(0) {}
  ~ThreadSafeQueue() = default;
  ThreadSafeQueue(const ThreadSafeQueue&) = delete;
  ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

  void Push(UniquePtr_T&& value) {
    queue.push(std::move(value));
    sem.release();
  }

  // Blocking Pop
  std::optional<UniquePtr_T> WaitAndPop() {
    sem.acquire();
    if (isDone) {
      sem.release();
      return std::nullopt;
    }
    UniquePtr_T value_ptr;
    while (!queue.try_pop(value_ptr));
    return std::move(value_ptr);
  }

  // Non-Blocking Pop
  bool TryPop(UniquePtr_T& value) {
    UniquePtr_T value_ptr;
    if (queue.try_pop(value_ptr)) {
      value = std::move(value_ptr);
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
      UniquePtr_T,
      xenium::policy::reclaimer<xenium::reclamation::hazard_pointer<>>>
      queue;
  std::counting_semaphore<> sem;
  std::atomic<bool> isDone;
};
