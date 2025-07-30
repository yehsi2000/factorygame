#include "Components/TimerComponent.h"

// Initialize the static ObjectPool for TimerInstance objects
ObjectPool<TimerInstance> TimerInstance::pool([]() {
    return std::make_unique<TimerInstance>();
});