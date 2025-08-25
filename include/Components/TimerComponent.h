#ifndef COMPONENTS_TIMERCOMPONENT_
#define COMPONENTS_TIMERCOMPONENT_

#include <array>
#include <cstdint>

// A handle to a timer instance. This is what components will store.
using TimerHandle = uint32_t;
constexpr TimerHandle INVALID_TIMER_HANDLE = 0;

/**
 * @brief An enum to identify the purpose of a timer.
 *
 */
enum class TimerId : int {
  Mine,
  AssemblingMachineCraft,
  MaxTimers // Represents the maximum number of timer types.
};

constexpr size_t MAX_TIMERS_PER_ENTITY =
    static_cast<size_t>(TimerId::MaxTimers);

struct TimerInstance {
  TimerId id;
  TimerHandle handle = INVALID_TIMER_HANDLE;
  float duration = 0.0f;
  float elapsed = 0.0f;
  bool isRepeating = false;
  bool isPaused = false;
  bool isActive = false;
};

struct TimerComponent {
  std::array<TimerHandle, MAX_TIMERS_PER_ENTITY> timers;
  TimerComponent() { timers.fill(INVALID_TIMER_HANDLE); }
};

/**
 * @brief A tag component added to an entity when one of its timers expires.
 * A tag component added to an entity when one of its timers expires.
 *
 */
struct TimerExpiredTag {
  TimerId expiredId;
};

#endif /* COMPONENTS_TIMERCOMPONENT_ */
