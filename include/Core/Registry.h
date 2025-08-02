#ifndef CORE_REGISTRY_
#define CORE_REGISTRY_

#include <algorithm>
#include <iostream>
#include <queue>
#include <string>
#include <typeinfo>

#include "Core/ComponentArray.h"
#include "Core/InputState.h"

constexpr int MAX_ENTITIES = 100000;

class Registry {
 private:
  std::queue<EntityID> availableEntities{};
  InputState inputState;

  uint32_t livingEntityCount = 0;

  std::unordered_map<const char*, std::shared_ptr<IComponentArray>>
      componentArrays{};

  template <typename T>
  const char* GetComponentTypeName() {
    return typeid(T).name();
  }

  template <typename T>
  std::shared_ptr<ComponentArray<T>> GetComponentArray() {
    const char* typeName = GetComponentTypeName<T>();
    assert(componentArrays.count(typeName) &&
           "Component type not registered before use.");
    return std::static_pointer_cast<ComponentArray<T>>(
        componentArrays[typeName]);
  }

  template <typename T>
  std::size_t GetComponentArraySize() {
    const char* typeName = GetComponentTypeName<T>();
    if (componentArrays.count(typeName) == 0) return 0;
    return componentArrays.at(typeName)->getSize();
  }

 public:
  Registry() {
    for (EntityID entity = 0; entity < MAX_ENTITIES; ++entity) {
      availableEntities.push(entity);
    }
  }

  EntityID CreateEntity() {
    assert(livingEntityCount < MAX_ENTITIES &&
           "Too many entities in existence.");
    EntityID id = availableEntities.front();
    availableEntities.pop();
    livingEntityCount++;
    std::cout << "Created entity with ID:" << id << std::endl;
    return id;
  }

  void DestroyEntity(EntityID entity) {
    assert(livingEntityCount > 0 && "Destroying non-existent entity.");

    for (auto const& pair : componentArrays) {
      pair.second->entityDestroyed(entity);
    }

    availableEntities.push(entity);
    livingEntityCount--;
  }

  // this should called only once
  template <typename T>
  void RegisterComponent() {
    const char* typeName = GetComponentTypeName<T>();
    if (componentArrays.find(typeName) == componentArrays.end()) {
      componentArrays[typeName] = std::make_shared<ComponentArray<T>>();
    }
  }

  template <typename T>
  void AddComponent(EntityID entity, T&& component) {
    GetComponentArray<T>()->addData(entity, std::move(component));
  }

  template <typename T, typename... Args>
  void EmplaceComponent(EntityID entity, Args&&... args) {
    GetComponentArray<T>()->emplaceData(entity, std::forward<Args>(args)...);
  }

  template <typename T>
  void RemoveComponent(EntityID entity) {
    GetComponentArray<T>()->removeData(entity);
  }

  template <typename T>
  T& GetComponent(EntityID entity) {
    return GetComponentArray<T>()->getData(entity);
  }

  template <typename T>
  bool HasComponent(EntityID entity) {
    const char* typeName = GetComponentTypeName<T>();
    auto it = componentArrays.find(typeName);
    if (it == componentArrays.end()) {
      return false;
    }
    return it->second->hasEntity(entity);
  }

  // Get all entities with given component
  template <typename... TComponent>
  std::vector<EntityID> view() {
    // No component
    if constexpr (sizeof...(TComponent) == 0) {
      return {};
    }

    // Get all component arrays
    std::vector<std::shared_ptr<IComponentArray>> arrays;
    (arrays.push_back(GetComponentArray<TComponent>()), ...);

    // Find smallest array
    std::sort(arrays.begin(), arrays.end(), [](const auto& a, const auto& b) {
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

  template <typename T, typename Func>
  void forEach(Func func) {
    GetComponentArray<T>()->forEach(func);
  }

  inline InputState& GetInputState() { return inputState; }
};

#endif /* CORE_REGISTRY_ */
