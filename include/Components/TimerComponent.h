#ifndef COMPONENTS_TIMERCOMPONENT_
#define COMPONENTS_TIMERCOMPONENT_

#include <array>
#include <chrono>
#include <functional>
#include <memory>
#include "ObjectPool.h"

enum class TimerId : int {
  Interact,
  MaxTimers  // 최대 개수 (배열 크기용)
};

constexpr size_t MAX_TIMERS_PER_ENTITY =
    static_cast<size_t>(TimerId::MaxTimers);  // enum 크기에 맞춤

struct TimerInstance {
  TimerId id;      // enum으로 POD 유지
  float duration;
  float elapsed = 0.0f;
  bool isRepeating = false;
  bool isPaused = false;
  bool isActive = true;  // 활성화 여부 (제거 대신 플래그로 관리, POD 유지)
  
  // Reset the timer for repeating
  void resetForRepeat() {
    if (isRepeating && isActive) {
      elapsed = 0.0f;  // Reset elapsed time for the next cycle
    }
  }
  
  // Static ObjectPool for TimerInstance objects
  static ObjectPool<TimerInstance> pool;
};

struct TimerComponent {
  std::array<TimerInstance, MAX_TIMERS_PER_ENTITY>
      timers;              // 고정 크기 배열: POD 친화적
  size_t activeCount = 0;
};

// 태그 컴포넌트: 만료 시 붙임 (빈 구조체 = POD)
struct TimerExpiredTag {
  TimerId expiredId;  // 어떤 타이머가 만료됐는지
};

#endif /* COMPONENTS_TIMERCOMPONENT_ */
