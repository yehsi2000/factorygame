#ifndef TIMERMANAGER_
#define TIMERMANAGER_

#include <vector>
#include "Components/TimerComponent.h" // For TimerInstance, TimerId, TimerHandle
#include "ObjectPool.h"
#include <memory>

// Manages the lifecycle of all TimerInstance objects in the game.
// This centralizes the logic and memory management.
class TimerManager {
public:
    TimerManager();

    // Creates a new timer and returns a handle to it.
    TimerHandle CreateTimer(TimerId id, float duration, bool isRepeating);

    // Retrieves a pointer to a timer instance from its handle.
    // Returns nullptr if the handle is invalid.
    TimerInstance* GetTimer(TimerHandle handle);

    // Destroys a timer instance, returning it to the pool.
    void DestroyTimer(TimerHandle handle);

private:
    // The pool that owns all the TimerInstance objects.
    ObjectPool<TimerInstance> timerPool;

    // A map from the public handle to the actual timer instance.
    // The manager owns the timer instance via the unique_ptr.
    std::vector<std::unique_ptr<TimerInstance>> handleToInstanceMap;
    
    // A queue of free handles to reuse for new timers, preventing handle reuse issues.
    std::vector<TimerHandle> freeHandles;
    
    // The next handle to assign if the free list is empty.
    TimerHandle nextHandle;
};

#endif /* TIMERMANAGER_ */
