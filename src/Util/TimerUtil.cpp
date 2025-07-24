 #include "Util/TimerUtil.h"
 
 namespace util {

 void AddTimer(TimerComponent& comp, TimerId id, float durSeconds, bool repeat = false) {
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

}