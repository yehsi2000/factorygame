#ifndef CORE_COMPONENTARRAY_
#define CORE_COMPONENTARRAY_

#include <cassert>
#include <cstddef>
#include <unordered_map>
#include <vector>

#include "Core/Entity.h"

class IComponentArray {
 public:
  virtual ~IComponentArray() = default;
  virtual void EntityDestroyed(EntityID entity) = 0;
  virtual bool HasEntity(EntityID entity) = 0;
  virtual std::size_t GetSize() = 0;
  virtual std::vector<EntityID> GetAllEntities() = 0;
};

// TODO : in case of bottleneck -> refactor to entt-style sparse map
// (using pagenation, tombstone, reverse iteration)
template <typename T>
class ComponentArray : public IComponentArray {
 private:
  // contiguous memory allocation for fast read
  // TODO : non-pod component's reallocation is expensive
  std::vector<T> componentArray;

  // entityID -> componentArray index
  std::unordered_map<EntityID, std::size_t> entityToIndexMap;
  // componentArray index -> entityID (for quick remove)
  std::unordered_map<std::size_t, EntityID> indexToEntityMap;

 public:
  void AddData(EntityID entity, T &&component) {
    assert(entityToIndexMap.find(entity) == entityToIndexMap.end() &&
           "Component added to same entity more than once.");

    std::size_t newIndex = componentArray.size();
    entityToIndexMap[entity] = newIndex;
    indexToEntityMap[newIndex] = entity;
    componentArray.emplace_back(std::move(component));
  }

  void RemoveData(EntityID entity) {
    assert(entityToIndexMap.find(entity) != entityToIndexMap.end() &&
           "Removing non-existent component.");

    std::size_t indexOfRemovedEntity = entityToIndexMap[entity];
    std::size_t indexOfLastElement = componentArray.size() - 1;
    componentArray[indexOfRemovedEntity] = std::move(componentArray[indexOfLastElement]);

    EntityID entityOfLastElement = indexToEntityMap[indexOfLastElement];
    entityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
    indexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

    componentArray.pop_back();
    entityToIndexMap.erase(entity);
    indexToEntityMap.erase(indexOfLastElement);
  }

  template <typename... Args>
  void EmplaceData(EntityID entity, Args &&...args) {
    assert(entityToIndexMap.find(entity) == entityToIndexMap.end() &&
           "Component added to same entity more than once.");
    std::size_t newIndex = componentArray.size();
    entityToIndexMap[entity] = newIndex;
    indexToEntityMap[newIndex] = entity;
    componentArray.emplace_back(std::forward<Args>(args)...);
  }

  T &GetData(EntityID entity) {
    assert(entityToIndexMap.find(entity) != entityToIndexMap.end() &&
           "Retrieving non-existent component.");
    return componentArray[entityToIndexMap[entity]];
  }

  template <typename Func>
  void forEach(Func func) {
    for (int i = static_cast<int>(componentArray.size()) - 1; i >= 0; --i) {
      func(indexToEntityMap.at(i), componentArray[i]);
    }
  }

  std::vector<EntityID> GetAllEntities() override {
    std::vector<EntityID> res;
    res.reserve(entityToIndexMap.size());
    for (auto &[id, _] : entityToIndexMap) {
      res.push_back(id);
    }
    return res;
  }

  bool HasEntity(EntityID entity) override {
    return entityToIndexMap.count(entity) > 0;
  }

  // Called when entity is destoryed
  void EntityDestroyed(EntityID entity) override {
    if (entityToIndexMap.count(entity)) {
      RemoveData(entity);
    }
  }

  std::size_t GetSize() override { return componentArray.size(); }
};

#endif /* CORE_COMPONENTARRAY_ */
