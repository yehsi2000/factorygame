#ifndef CORE_TIMERMANAGER_
#define CORE_TIMERMANAGER_

#include <memory>
#include <vector>

#include "Components/TimerComponent.h"  // For TimerInstance, TimerId, TimerHandle
#include "Core/ObjectPool.h"

/**
 * @brief Manages the lifecycle of all TimerInstance objects in the game, centralizing the logic and memory management. 
 */
class TimerManager {
 public:
  TimerManager();

  /**
   * @brief Creates a new timer and returns a handle to it.
   * 
   * @param id TimerId to represent purpose of is timer.
   * @param duration How long does this timer take to expire.
   * @param bIsRepeating If true, repeat timer after it's expiration.
   * @return TimerHandle 
   */
  TimerHandle CreateTimer(TimerId id, float duration, bool bIsRepeating);

  /**
   * @brief Retrieves a pointer to a timer instance from its handle.
   * 
   * @param handle Timer identifier
   * @return TimerInstance* nullptr if the handle is invalid.
   */
  TimerInstance* GetTimer(TimerHandle handle);

  /**
   * @brief Destroys a timer instance, returning it to the pool.
   * 
   * @param handle Handle of timer to destroy
   */
  void DestroyTimer(TimerHandle handle);

 private:
  // The pool that owns all the TimerInstance objects.
  ObjectPool<TimerInstance> timerPool;

  /**
   * @brief A map from the public handle to the actual timer instance.
   * The manager owns the timer instance via the unique_ptr.
   */
  std::vector<std::unique_ptr<TimerInstance>> handleToInstanceMap;

  /**
   * @brief A queue of free handles to reuse for new timers, preventing handle reuse issues.
   * 
   */
  std::vector<TimerHandle> freeHandles;

  // 
  /**
   * @brief The next handle to assign if the free list is empty.
   * 
   */
  TimerHandle nextHandle;
};

#endif /* CORE_TIMERMANAGER_ */
