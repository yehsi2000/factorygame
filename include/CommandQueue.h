#ifndef __COMMANDQUEUE__
#define __COMMANDQUEUE__

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>

#include "Event.h"

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

  template <typename EventType>
  void PushEvent(EventDispatcher* dispatcher, EventType&& event) {
    Push([dispatcher, event = std::forward<EventType>(event)]() {
      dispatcher->Dispatch(event);
    });
  }

  std::function<void()> Pop() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&] { return !queue.empty(); });
    auto cmd = std::move(queue.front());
    queue.pop();
    return cmd;
  }

  std::queue<std::function<void()>> PopAll() {
    std::lock_guard<std::mutex> lock(mtx);
    std::queue<std::function<void()>> all_commands;
    std::swap(all_commands, queue);  // 현재 큐와 빈 큐를 교체
    return all_commands;
  }

  bool Empty() {
    std::lock_guard<std::mutex> lock(mtx);
    return queue.empty();
  }
};

#endif