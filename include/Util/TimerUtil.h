#ifndef UTIL_TIMERUTIL_
#define UTIL_TIMERUTIL_

#include "Components/TimerComponent.h"  // For TimerId
#include "Core/Entity.h"

class Registry;
class TimerManager;

namespace util {

/**
 * @brief Attaches a new timer to an entity.
 *
 * This function creates a timer in the TimerManager and attaches its handle
 * to the entity's TimerComponent. If the entity does not have a TimerComponent,
 * one will be added automatically. If a timer of the same TimerId already
 * exists on the entity, it will be detached first before the new one is
 * attached.
 *
 * @param registry The game's entity-component registry.
 * @param timerManager The game's timer manager.
 * @param entity The ID of the entity to attach the timer to.
 * @param id The semantic ID of the timer (e.g., TimerId::Interact).
 * @param duration The duration of the timer in seconds.
 * @param bIsRepeating Whether the timer should repeat after expiring.
 */
void AttachTimer(Registry* registry, TimerManager* timerManager,
                 EntityID entity, TimerId id, float duration, bool bIsRepeating);

/**
 * @brief Detaches a timer from an entity.
 *
 * This function finds the timer with the given ID on the entity, destroys it
 * in the TimerManager, and removes its handle from the TimerComponent.
 *
 * @param registry The game's entity-component registry.
 * @param timerManager The game's timer manager.
 * @param entity The ID of the entity to detach the timer from.
 * @param id The semantic ID of the timer to detach.
 */
void DetachTimer(Registry* registry, TimerManager* timerManager,
                 EntityID entity, TimerId id);

}  // namespace util

#endif /* UTIL_TIMERUTIL_ */
