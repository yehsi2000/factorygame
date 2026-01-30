#pragma once
 
#include "Core/SystemContext.h"

class EventDispatcher;
class CommandQueue;
class Registry;
class TimerManager;

/**
 * @brief Check expired timer and execute corresponding logic 
 * for each kind of TimerID
 *
 */
class TimerExpireSystem {
  EventDispatcher* eventDispatcher;
  CommandQueue* commandQueue;
  Registry* registry;
  TimerManager* timerManager;

 public:
  explicit TimerExpireSystem(const SystemContext& context);
  ~TimerExpireSystem();
  void Update();
};
