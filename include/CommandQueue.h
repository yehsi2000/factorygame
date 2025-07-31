#ifndef COMMANDQUEUE_
#define COMMANDQUEUE_

#include <memory>
#include <queue>
#include "Command.h"

class CommandQueue {
  std::queue<std::unique_ptr<Command>> queue;

 public:
  void Enqueue(std::unique_ptr<Command> cmd) {
    queue.push(std::move(cmd));
  }

  std::unique_ptr<Command> Dequeue() {
    if (queue.empty()) {
      return nullptr;
    }
    auto cmd = std::move(queue.front());
    queue.pop();
    return cmd;
  }

  bool IsEmpty() const {
    return queue.empty();
  }
};

#endif /* COMMANDQUEUE_ */
