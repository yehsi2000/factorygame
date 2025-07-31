#include "TimerManager.h"
#include <cassert>

TimerManager::TimerManager() 
    : timerPool([]() { return std::make_unique<TimerInstance>(); }),
      nextHandle(1) // Start handles at 1, as 0 is invalid.
{
    // Pre-allocate some space to avoid frequent reallocations.
    handleToInstanceMap.resize(128); 
    timerPool.PreAllocate(50); // Pre-warm the pool
}

TimerHandle TimerManager::CreateTimer(TimerId id, float duration, bool isRepeating) {
    TimerHandle handle;

    // Reuse a handle from the free list if available.
    if (!freeHandles.empty()) {
        handle = freeHandles.back();
        freeHandles.pop_back();
    } else {
        handle = nextHandle++;
        // Resize the map if we've run out of pre-allocated space.
        if (handle >= handleToInstanceMap.size()) {
            handleToInstanceMap.resize(handleToInstanceMap.size() * 2);
        }
    }

    auto timer = timerPool.Acquire();
    assert(timer != nullptr && "Timer pool ran out of objects!");

    // Initialize the timer instance.
    timer->id = id;
    timer->handle = handle;
    timer->duration = duration;
    timer->isRepeating = isRepeating;
    timer->elapsed = 0.0f;
    timer->isPaused = false;
    timer->isActive = true;

    handleToInstanceMap[handle] = std::move(timer);
    return handle;
}

TimerInstance* TimerManager::GetTimer(TimerHandle handle) {
    if (handle == INVALID_TIMER_HANDLE || handle >= handleToInstanceMap.size() || !handleToInstanceMap[handle]) {
        return nullptr;
    }
    return handleToInstanceMap[handle].get();
}

void TimerManager::DestroyTimer(TimerHandle handle) {
    if (handle == INVALID_TIMER_HANDLE || handle >= handleToInstanceMap.size() || !handleToInstanceMap[handle]) {
        return;
    }

    auto timer = std::move(handleToInstanceMap[handle]);
    if (timer) {
        timer->isActive = false; // Mark as inactive.
        timerPool.Release(std::move(timer));
        // The unique_ptr in the map is now null after the move.
        freeHandles.push_back(handle); // Add the handle to the free list for reuse.
    }
}
