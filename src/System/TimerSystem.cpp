#include "System/TimerSystem.h"

#include "Components/TimerComponent.h"
#include "Registry.h"
#include "Util/TimerUtil.h"

void TimerSystem::Update(float dt) {
  auto view = registry->view<TimerComponent>();
  for (auto entity : view) {
    auto& timerComp = registry->GetComponent<TimerComponent>(entity);
    for (auto& timer : timerComp.timers) {  // 배열 순회 (고정 크기라 빠름)
      if (!timer.isActive || timer.isPaused) continue;
      timer.elapsed += dt;
      if (timer.elapsed >= timer.duration) {
        registry->EmplaceComponent<TimerExpiredTag>(entity,
                                                    TimerExpiredTag{timer.id});
        if (timer.isRepeating) {
          util::ResetTimerForRepeat(timer);  // Use the new utility function
        } else {
          util::RemoveTimer(timerComp, timer.id);
        }
      }
    }
    // 모든 타이머 비활성 컴포넌트 제거 (오류시 deferred 방식으로 전환)
    if (timerComp.activeCount == 0) {
      registry->RemoveComponent<TimerComponent>(entity);
    }
  }
}

bool TimerSystem::ScheduleEvent(std::shared_ptr<const Event> event, float delaySeconds) {
  return timerManager.ScheduleEvent(event, delaySeconds);
}

void TimerSystem::ProcessScheduledEvents() {
  ProcessScheduledEventsBatch(100); // Use batch processing with default batch size
}

void TimerSystem::ProcessScheduledEventsBatch(size_t batchSize) {
  auto readyEvents = timerManager.ExtractReadyEventsBatch(batchSize);
  
  // In a real implementation, we would process these events
  // For now, we're just extracting them
  (void)readyEvents; // Suppress unused variable warning
}

TimerManagerMetrics TimerSystem::GetMetrics() const {
  return timerManager.GetMetrics();
}

void TimerSystem::ResetMetrics() {
  timerManager.ResetMetrics();
}

void TimerSystem::SetMaxQueueSize(int maxSize) {
  timerManager.SetMaxQueueSize(maxSize);
}