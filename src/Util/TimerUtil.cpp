#include "Util/TimerUtil.h"
#include "Registry.h"
  
namespace util {

 void AddTimer(TimerComponent& comp, TimerId id, float durSeconds, bool repeat) {
  auto& timers = comp.timers;
  auto& activeCount = comp.activeCount;
  size_t index = static_cast<size_t>(id);
  if (index >= MAX_TIMERS_PER_ENTITY) return;
  auto& timer = timers[index];
  timer.id = id;
  timer.duration = durSeconds;
  timer.elapsed = 0.0f;
  timer.isRepeating = repeat;
  timer.isPaused = false;
  timer.isActive = true;
  ++activeCount;
 }

 void RemoveTimer(TimerComponent &comp, TimerId id) {
  auto& timers = comp.timers;
  auto& activeCount = comp.activeCount;
  size_t index = static_cast<size_t>(id);
  if (index < MAX_TIMERS_PER_ENTITY) {
    timers[index].isActive = false;
    --activeCount;
  }
}

void ResetTimerForRepeat(TimerInstance& timer) {
  if (timer.isRepeating && timer.isActive) {
    timer.elapsed = 0.0f;  // Reset elapsed time for the next cycle
  }
}

bool IsTimerExpired(const TimerInstance& timer) {
  return timer.isActive && !timer.isPaused && timer.elapsed >= timer.duration;
}

void CreateTimerOnEntity(Registry* registry, EntityID entity, TimerId id, float duration, bool repeat) {
  // Check if the entity already has a TimerComponent, if not, add one
  if (!registry->HasComponent<TimerComponent>(entity)) {
    registry->EmplaceComponent<TimerComponent>(entity, TimerComponent{});
  }
  
  // Get the TimerComponent and add the timer to it
  auto& timerComp = registry->GetComponent<TimerComponent>(entity);
  util::AddTimer(timerComp, id, duration, repeat);
}

}
