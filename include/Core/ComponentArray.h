#pragma once
 
#include <cassert>
#include <cstddef>
#include <unordered_map>
#include <vector>

#include "Core/Entity.h"

/**
 * @brief Interface for component arrays.
 * @details Provides a common interface for type-erased storage of components.
 * This allows the Registry to manage component arrays of different types
 * without knowing their specific template arguments.
 */
class IComponentArray {
 public:
  virtual ~IComponentArray() = default;
  virtual void EntityDestroyed(Entity entity) = 0;
  virtual bool HasEntity(Entity entity) = 0;
  virtual std::size_t GetSize() = 0;
  virtual std::vector<Entity> GetAllEntities() = 0;
};

// TODO : in case of bottleneck -> refactor to entt-style sparse map
// (using pagenation, tombstone, reverse iteration)
/**
 * @brief A cache-friendly container for a single type of component.
 * @details Stores components of a specific type in a contiguous array for fast
 * iteration. It uses a map to link entity IDs to their component's index in
 * the array, allowing for efficient addition, removal, and access of
 * components.
 * @tparam T The type of component to store.
 */
template <typename T>
class ComponentArray : public IComponentArray {
 private:
  // contiguous memory allocation for fast read
  // TODO : non-pod component's reallocation is expensive
  std::vector<T> componentArray;

  // entityID -> componentArray index
  std::unordered_map<Entity, std::size_t> entityToIndexMap;
  // componentArray index -> entityID (for quick remove)
  std::unordered_map<std::size_t, Entity> indexToEntityMap;

 public:
  void AddData(Entity entity, T &&component) {
    assert(entityToIndexMap.find(entity) == entityToIndexMap.end() &&
           "Component added to same entity more than once.");

    std::size_t newIndex = componentArray.size();
    entityToIndexMap[entity] = newIndex;
    indexToEntityMap[newIndex] = entity;
    componentArray.emplace_back(std::move(component));
  }

  void RemoveData(Entity entity) {
    assert(entityToIndexMap.find(entity) != entityToIndexMap.end() &&
           "Removing non-existent component.");

    std::size_t indexOfRemovedEntity = entityToIndexMap[entity];
    std::size_t indexOfLastElement = componentArray.size() - 1;
    componentArray[indexOfRemovedEntity] = std::move(componentArray[indexOfLastElement]);

    Entity entityOfLastElement = indexToEntityMap[indexOfLastElement];
    entityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
    indexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

    componentArray.pop_back();
    entityToIndexMap.erase(entity);
    indexToEntityMap.erase(indexOfLastElement);
  }

  template <typename... Args>
  void EmplaceData(Entity entity, Args &&...args) {
    assert(entityToIndexMap.find(entity) == entityToIndexMap.end() &&
           "Component added to same entity more than once.");
    std::size_t newIndex = componentArray.size();
    entityToIndexMap[entity] = newIndex;
    indexToEntityMap[newIndex] = entity;
    componentArray.emplace_back(std::forward<Args>(args)...);
  }

  T &GetData(Entity entity) {
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

  std::vector<Entity> GetAllEntities() override {
    std::vector<Entity> res;
    res.reserve(entityToIndexMap.size());
    for (auto &[id, _] : entityToIndexMap) {
      res.push_back(id);
    }
    return res;
  }

  bool HasEntity(Entity entity) override {
    return entityToIndexMap.count(entity) > 0;
  }

  // Called when entity is destoryed
  void EntityDestroyed(Entity entity) override {
    if (entityToIndexMap.count(entity)) {
      RemoveData(entity);
    }
  }

  std::size_t GetSize() override { return componentArray.size(); }
};
