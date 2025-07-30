#ifndef SYSTEM_TIMERSYSTEM_
#define SYSTEM_TIMERSYSTEM_

#include "Entity.h"
#include "Registry.h"
#include "TimerManager.h"
#include <memory>
#include "Event.h"

class TimerSystem {
  Registry* registry;
  TimerManager timerManager;  // TimerManager for delayed event scheduling

 public:
  TimerSystem(Registry* r) : registry(r) {}
  
  // Schedule an event to be executed after a delay in seconds
  bool ScheduleEvent(std::shared_ptr<const Event> event, float delaySeconds);
  
  // Process events that are ready to execute
  void ProcessScheduledEvents();
  
  // Process events in batch mode for better performance
  void ProcessScheduledEventsBatch(size_t batchSize = 100);
  
  // Get performance metrics
  TimerManagerMetrics GetMetrics() const;
  
  // Reset performance metrics
  void ResetMetrics();
  
  // Set maximum queue size for overflow prevention
  void SetMaxQueueSize(int maxSize);

  void Update(float dt);
};

#endif /* SYSTEM_TIMERSYSTEM_ */
