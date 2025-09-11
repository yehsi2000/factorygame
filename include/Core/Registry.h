#ifndef CORE_REGISTRY_
#define CORE_REGISTRY_

#include <algorithm>
#include <iostream>
#include <queue>
#include <memory>

#include "Core/ComponentArray.h"
#include "Core/EventDispatcher.h"

constexpr int MAX_ENTITIES = 1000000;

/**
 * @brief The core of the Entity-Component-System (ECS) architecture.
 * @details Manages the lifecycle of all entities and the storage of their
 *          components. Systems use the Registry to query for entities that
 *          possess specific sets of components.
 */
class Registry {
 private:
  std::queue<EntityID> availableEntities{};

  uint32_t livingEntityCount = 0;
  std::size_t COMPONENT_ID = 0;

  std::vector<std::unique_ptr<IComponentArray>> componentArrays{};
  EventDispatcher* eventDispatcher;

  template <typename T>
  std::size_t GetComponentTypeID() {
    const static std::size_t typeID = COMPONENT_ID++;
    return typeID;
  }

  template <typename T>
  ComponentArray<T> *GetComponentArray() {
    std::size_t compTypeId = GetComponentTypeID<T>();
    if (componentArrays.size() <= compTypeId) {
      std::cerr << "Assertion failed: Component type '" << typeid(T).name()
                << "' not registered before use." << std::endl;
      std::abort();
    }
    return static_cast<ComponentArray<T> *>(componentArrays[compTypeId].get());
  }

  template <typename T>
  std::size_t GetComponentArraySize() {
    std::size_t compTypeId = GetComponentTypeID<T>();
    if (componentArrays.size() <= compTypeId) return 0;
    return componentArrays[compTypeId]->GetSize();
  }

 public:
  Registry(EventDispatcher* dispatcher) : eventDispatcher(dispatcher) {
    for (EntityID entity = 1; entity < MAX_ENTITIES; ++entity) {
      availableEntities.push(entity);
    }
  }

  /**
   * @brief Creates a new entity.
   * @details Acquires a unique EntityID from the pool of available IDs.
   * @return The ID of the newly created entity.
   */
  EntityID CreateEntity() {
    assert(livingEntityCount < MAX_ENTITIES &&
           "Too many entities in existence.");
    EntityID id = availableEntities.front();
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
  void DestroyEntity(EntityID entity) {
    assert(livingEntityCount > 0 && "Destroying non-existent entity.");

    eventDispatcher->Publish(EntityDestroyedEvent(entity));

    for (auto& compArray : componentArrays) {
      compArray->EntityDestroyed(entity);
    }

    availableEntities.push(entity);
    livingEntityCount--;
  }

  /**
   * @brief Registers a new component type with the registry.
   * @details This must be called once for each component type before it can be
   *          added to entities.
   * @tparam T The component type to register.
   */
  template <typename T>
  void RegisterComponent() {
    std::size_t compTypeId = GetComponentTypeID<T>();
    if (componentArrays.size() <= compTypeId) {
      componentArrays.push_back(std::make_unique<ComponentArray<T>>());
    }
  }

  /**
   * @brief Adds a component to an entity.
   * @tparam T The component type.
   * @param entity The target entity's ID.
   * @param component The component instance to add.
   */
  template <typename T>
  void AddComponent(EntityID entity, T &&component) {
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
  void EmplaceComponent(EntityID entity, Args &&...args) {
    GetComponentArray<T>()->EmplaceData(entity, std::forward<Args>(args)...);
  }

  /**
   * @brief Removes a component from an entity.
   * @tparam T The component type to remove.
   * @param entity The target entity's ID.
   */
  template <typename T>
  void RemoveComponent(EntityID entity) {
    GetComponentArray<T>()->RemoveData(entity);
  }

  /**
   * @brief Retrieves a reference to an entity's component.
   * @tparam T The component type to retrieve.
   * @param entity The target entity's ID.
   * @return A reference to the component.
   */
  template <typename T>
  T &GetComponent(EntityID entity) {
    return GetComponentArray<T>()->GetData(entity);
  }

  /**
   * @brief Checks if an entity has a specific component.
   * @tparam T The component type to check for.
   * @param entity The target entity's ID.
   * @return True if the entity has the component, false otherwise.
   */
  template <typename T>
  bool HasComponent(EntityID entity) {
    std::size_t compTypeId = GetComponentTypeID<T>();
    if (componentArrays.size() <= compTypeId)
      return false;
    else 
      return componentArrays[compTypeId]->HasEntity(entity);
    
  }

  /**
   * @brief Creates a view of all entities that have a given set of components.
   * @details This is the primary method for systems to query entities. It
   *          returns a vector of entity IDs that can be iterated upon.
   * @tparam TComponent The component types required for an entity to be
   * included.
   * @return A vector of EntityIDs matching the query.
   */
  template <typename... TComponent>
  std::vector<EntityID> view() {
    // No component
    if constexpr (sizeof...(TComponent) == 0) {
      return {};
    }

    // Get all component arrays
    std::vector<IComponentArray *> arrays;
    (arrays.push_back(GetComponentArray<TComponent>()), ...);

    // Find smallest array
    std::sort(arrays.begin(), arrays.end(), [](const auto &a, const auto &b) {
      return a->GetSize() < b->GetSize();
    });
    std::vector<EntityID> result = arrays[0]->GetAllEntities();

    // Prune entities which doesn't have all components passed
    for (size_t i = 1; i < arrays.size(); ++i) {
      result.erase(std::remove_if(result.begin(), result.end(),
                                  [&](EntityID entity) {
                                    return !arrays[i]->HasEntity(entity);
                                  }),
                   result.end());
    }

    return result;
  }

  /**
   * @brief Iterates over all entities with a specific component.
   * @tparam T The component type to iterate over.
   * @tparam Func The function or lambda to execute for each component.
   * @param func The function to call, which receives the component as an
   * argument.
   */
  template <typename T, typename Func>
  void forEach(Func func) {
    GetComponentArray<T>()->forEach(func);
  }
};

#endif/* CORE_REGISTRY_ */
