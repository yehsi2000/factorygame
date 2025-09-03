#ifndef CORE_COMMANDQUEUE_
#define CORE_COMMANDQUEUE_

#include <memory>
#include <queue>

#include "Commands/Command.h"

/**
 * @brief A queue for deferred execution of commands.
 * @details This class provides a thread-safe way to enqueue commands from various
 *          systems. The main engine loop dequeues and executes these commands at a
 *          safe point in the frame, preventing race conditions and ensuring
 *          deterministic state changes.
 */
class CommandQueue {
  std::queue<std::unique_ptr<Command>> queue;

 public:
  void Enqueue(std::unique_ptr<Command> cmd) { queue.push(std::move(cmd)); }

  std::unique_ptr<Command> Dequeue() {
    if (queue.empty()) {
      return nullptr;
    }
    auto cmd = std::move(queue.front());
    queue.pop();
    return cmd;
  }

  bool IsEmpty() const { return queue.empty(); }
};

#endif /* CORE_COMMANDQUEUE_ */
