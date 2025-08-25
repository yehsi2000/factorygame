#ifndef CORE_REGISTRY_
#define CORE_REGISTRY_

#include <algorithm>
#include <iostream>
#include <queue>

#include "Core/ComponentArray.h"
#include "Core/InputState.h"

constexpr int MAX_ENTITIES = 100000;

/**
 * @brief The core of the Entity-Component-System (ECS) architecture.
 * @details Manages the lifecycle of all entities and the storage of their
 *          components. Systems use the Registry to query for entities that
 *          possess specific sets of components.
 */
class Registry {
 private:
  std::queue<EntityID> availableEntities{};
  InputState inputState;

  uint32_t livingEntityCount = 0;

  std::unordered_map<const char *, std::unique_ptr<IComponentArray>>
      componentArrays{};

  template <typename T>
  const char *GetComponentTypeName() {
    return typeid(T).name();
  }

  template <typename T>
  ComponentArray<T> *GetComponentArray() {
    const char *typeName = GetComponentTypeName<T>();
    if (componentArrays.count(typeName) == 0) {
      std::cerr << "Assertion failed: Component type '" << typeName
                << "' not registered before use." << std::endl;
      std::abort();
    }
    return static_cast<ComponentArray<T> *>(componentArrays[typeName].get());
  }

  template <typename T>
  std::size_t GetComponentArraySize() {
    const char *typeName = GetComponentTypeName<T>();
    if (componentArrays.count(typeName) == 0) return 0;
    return componentArrays.at(typeName)->getSize();
  }

 public:
  Registry() {
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

    for (auto const &pair : componentArrays) {
      pair.second->entityDestroyed(entity);
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
    const char *typeName = GetComponentTypeName<T>();
    if (componentArrays.find(typeName) == componentArrays.end()) {
      componentArrays[typeName] = std::make_unique<ComponentArray<T>>();
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
    GetComponentArray<T>()->addData(entity, std::move(component));
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
    GetComponentArray<T>()->emplaceData(entity, std::forward<Args>(args)...);
  }

  /**
   * @brief Removes a component from an entity.
   * @tparam T The component type to remove.
   * @param entity The target entity's ID.
   */
  template <typename T>
  void RemoveComponent(EntityID entity) {
    GetComponentArray<T>()->removeData(entity);
  }

  /**
   * @brief Retrieves a reference to an entity's component.
   * @tparam T The component type to retrieve.
   * @param entity The target entity's ID.
   * @return A reference to the component.
   */
  template <typename T>
  T &GetComponent(EntityID entity) {
    return GetComponentArray<T>()->getData(entity);
  }

  /**
   * @brief Checks if an entity has a specific component.
   * @tparam T The component type to check for.
   * @param entity The target entity's ID.
   * @return True if the entity has the component, false otherwise.
   */
  template <typename T>
  bool HasComponent(EntityID entity) {
    const char *typeName = GetComponentTypeName<T>();
    auto it = componentArrays.find(typeName);
    if (it == componentArrays.end()) {
      return false;
    }
    return it->second->hasEntity(entity);
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
      return a->getSize() < b->getSize();
    });
    std::vector<EntityID> result = arrays[0]->getAllEntities();

    // Prune entities which doesn't have all components passed
    for (size_t i = 1; i < arrays.size(); ++i) {
      result.erase(std::remove_if(result.begin(), result.end(),
                                  [&](EntityID entity) {
                                    return !arrays[i]->hasEntity(entity);
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

  inline InputState &GetInputState() { return inputState; }
};

#endif /* CORE_REGISTRY_ */
