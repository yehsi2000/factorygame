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
          timer.elapsed -= timer.duration;
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