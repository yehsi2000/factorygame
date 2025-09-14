#ifndef CORE_ENTITY_
#define CORE_ENTITY_

/**
 * @brief A unique identifier for an entity in the game world.
 * @details In the ECS architecture, an entity is simply a lightweight ID. It
 * acts as a key to associate various components that define its properties and
 * behavior.
 */
using EntityID = unsigned long long;

/**
 * @brief A constant representing an invalid or null entity.
 * @details Used to indicate the absence of a valid entity, such as when a
 * search finds no result or a reference is uninitialized.
 */
constexpr EntityID INVALID_ENTITY = 0;

#endif /* CORE_ENTITY_ */
