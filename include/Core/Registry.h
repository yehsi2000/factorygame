#pragma once

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <type_traits>

#include "Core/ComponentArray.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"

constexpr long long MAX_ENTITIES = 1000000;

template <typename T>
struct NotTriviallyDefaultConstructable;

template <typename T>
struct NotTriviallyCopyable;

template <typename T>
struct NotTriviallyDestructible;

/**
 * @brief The core of the Entity-Component-System (ECS) architecture.
 * @details Manages the lifecycle of all entities and the storage of their
 *          components. Systems use the Registry to query for entities that
 *          possess specific sets of components.
 */
class Registry {
 private:
  std::queue<Entity> availableEntities{};
  std::shared_mutex compArrayMutex;

  uint32_t livingEntityCount = 0;
  uint32_t entityIdTop = 0;
  std::atomic<std::size_t> typeCounter = 0;

  std::vector<std::unique_ptr<IComponentArray>> componentArrays{};
  EventDispatcher *eventDispatcher;

  template <typename T>
  std::size_t GetComponentTypeID() {
    const static std::size_t typeID = typeCounter++;
    return typeID;
  }

  template <typename T>
  ComponentArray<T> *GetComponentArray() {
    std::size_t compTypeId = GetComponentTypeID<T>();

    // get componentarray pointer if it exists
    {
      std::shared_lock<std::shared_mutex> readLock(compArrayMutex);
      if (compTypeId < componentArrays.size() &&
          componentArrays[compTypeId] != nullptr) {
        return static_cast<ComponentArray<T> *>(
            componentArrays[compTypeId].get());
      }
    }

    // component is not registered
    {
      std::unique_lock<std::shared_mutex> writeLock(compArrayMutex);

      // check once more inside exclusive lock
      if (compTypeId < componentArrays.size() &&
          componentArrays[compTypeId] != nullptr) {
        return static_cast<ComponentArray<T> *>(
            componentArrays[compTypeId].get());
      }

      // It really doesn't exist

      // check if it's triviality for performance

      // if constexpr (!std::is_trivially_default_constructible_v<T>) {
      //   NotTriviallyCopyable<T> t;
      // }

      // if constexpr (!std::is_trivially_copyable_v<T>) {
      //   NotTriviallyCopyable<T> t;
      // }
      // if constexpr (!std::is_trivially_destructible_v<T>) {
      //   NotTriviallyDestructible<T> t;
      // }

      // expand the componentArray container
      if (componentArrays.size() <= compTypeId) {
        componentArrays.resize(compTypeId + 1);
      }

      // add unique_ptr of component array of that component type
      if (componentArrays[compTypeId] == nullptr) {
        componentArrays[compTypeId] = std::make_unique<ComponentArray<T>>();
      }

      return static_cast<ComponentArray<T> *>(
          componentArrays[compTypeId].get());
    }
  }

 public:
  Registry(EventDispatcher *dispatcher) : eventDispatcher(dispatcher) {}

  /**
   * @brief Creates a new entity.
   * @details Acquires a unique Entity from the pool of available IDs.
   * @return The ID of the newly created entity.
   */
  Entity CreateEntity() {
    // TODO : this should also be refactored with constructor
    // Should allocated unused number and take destroyed entity number in O(1)
    assert(livingEntityCount < MAX_ENTITIES &&
           "Too many entities in existence.");
    if (availableEntities.empty()) {
      availableEntities.push(Entity(++entityIdTop));
    }
    Entity id = availableEntities.front();
    availableEntities.pop();
    livingEntityCount++;
    return id;
  }

  /**
   * @brief Destroys an entity.
   * @details Removes all components associated with the entity and returns its
   *          ID to the available pool.
   * @param entity The ID of the entity to destroy.
   */
  void DestroyEntity(Entity entity) {
    assert(livingEntityCount > 0 && "Destroying non-existent entity.");

    eventDispatcher->Publish(EntityDestroyedEvent(entity));

    for (auto &compArray : componentArrays) {
      if (compArray == nullptr) continue;
      compArray->EntityDestroyed(entity);
    }

    entity.generation++;
    availableEntities.push(entity);
    livingEntityCount--;
  }

  /**
   * @brief Adds a component to an entity.
   * @tparam T The component type.
   * @param entity The target entity's ID.
   * @param component The component instance to add.
   */
  template <typename T>
  void AddComponent(Entity entity, T &&component) {
    GetComponentArray<T>()->AddData(entity, std::move(component));
  }

  /**
   * @brief Constructs a component in-place for an entity.
   * @tparam T The component type.
   * @tparam Args The types of arguments for the component's constructor.
   * @param entity The target entity's ID.
   * @param args The arguments to forward to the component's constructor.
   */
  template <typename T, typename... Args>
  void EmplaceComponent(Entity entity, Args &&...args) {
    GetComponentArray<T>()->EmplaceData(entity, std::forward<Args>(args)...);
  }

  /**
   * @brief Removes a component from an entity.
   * @tparam T The component type to remove.
   * @param entity The target entity's ID.
   */
  template <typename T>
  void RemoveComponent(Entity entity) {
    GetComponentArray<T>()->RemoveData(entity);
  }

  /**
   * @brief Retrieves a reference to an entity's component.
   * @tparam T The component type to retrieve.
   * @param entity The target entity's ID.
   * @return A reference to the component.
   */
  template <typename T>
  T &GetComponent(Entity entity) {
    return GetComponentArray<T>()->GetData(entity);
  }

  /**
   * @brief Checks if an entity has a specific component.
   * @tparam T The component type to check for.
   * @param entity The target entity's ID.
   * @return True if the entity has the component, false otherwise.
   */
  template <typename T>
  bool HasComponent(Entity entity) {
    std::size_t compTypeId = GetComponentTypeID<T>();

    if (componentArrays.size() <= compTypeId) return false;

    if (componentArrays[compTypeId] == nullptr) return false;

    return componentArrays[compTypeId]->HasEntity(entity);
  }

  /**
   * @brief Creates a view of all entities that have a given set of components.
   * @details This is the primary method for systems to query entities. It
   *          returns a vector of entity IDs that can be iterated upon.
   * @tparam TComponent The component types required for an entity to be
   * included.
   * @return A vector of Entitys matching the query.
   */
  template <typename... TComponent>
  std::vector<Entity> view() {
    // No component
    if constexpr (sizeof...(TComponent) == 0) {
      return {};
    }

    // Get all component arrays
    std::vector<IComponentArray *> arrays;
    (arrays.push_back(GetComponentArray<TComponent>()), ...);

    // Find smallest array
    auto minArray = std::min_element(arrays.begin(), arrays.end(), [](const auto &a, const auto &b) {
      return a->GetSize() < b->GetSize();
    });

    std::vector<Entity> result = (*minArray)->GetAllEntities();

    // Prune entities which doesn't have all components passed
    for (size_t i = 1; i < arrays.size(); ++i) {
      result.erase(std::remove_if(result.begin(), result.end(),
                                  [&](Entity entity) {
                                    return !arrays[i]->HasEntity(entity);
                                  }),
                   result.end());
    }

    return result;
  }
};
