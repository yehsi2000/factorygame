#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
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
  virtual bool HasEntity(Entity entity) const = 0;
  virtual std::size_t GetSize() const = 0;
  virtual std::vector<Entity> GetAllEntities() = 0;
};

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
  static constexpr std::size_t PAGE_SIZE =
      4096;  // Pagination for memory reduction
  static constexpr uint32_t TOMBSTONE =
      std::numeric_limits<uint32_t>::max();  // invalid value

  std::vector<std::unique_ptr<std::array<uint32_t, PAGE_SIZE>>>
      sparse;  // entity id -> dense index
  std::vector<Entity> dense;
  std::vector<T> componentArray;

  uint32_t *GetPage(std::size_t pageIdx) const {
    // page out of bound
    if (pageIdx >= sparse.size() || !sparse[pageIdx]) {
      return nullptr;
    }
    return sparse[pageIdx]->data();
  }

  uint32_t *GetPageRequired(std::size_t pageIdx) {
    // page out of bound
    if (pageIdx >= sparse.size()) {
      sparse.resize(pageIdx + 1);
    }
    // in-range but page does not exists
    if (!sparse[pageIdx]) {
      sparse[pageIdx] =
          std::make_unique<std::array<uint32_t, PAGE_SIZE>>();  // new page
      sparse[pageIdx]->fill(TOMBSTONE);  // invalidate all entries
    }

    return sparse[pageIdx]->data();
  }

 public:
  bool HasEntity(Entity entity) const override {
    std::size_t page = entity.Id() / PAGE_SIZE;
    std::size_t offset = entity.Id() % PAGE_SIZE;
    auto *p = GetPage(page);
    if (!p) return false;
    uint32_t idx = p[offset];
    return idx < dense.size() && dense[idx] == entity;
  }

  void AddData(Entity entity, T &&component) {
    // assert(entityToIndexMap.find(entity) == entityToIndexMap.end() &&
    //        "Component added to same entity more than once.");
    std::size_t page = entity.Id() / PAGE_SIZE;
    std::size_t offset = entity.Id() % PAGE_SIZE;
    auto *p = GetPageRequired(page);
    p[offset] = static_cast<uint32_t>(dense.size());
    dense.push_back(entity);
    componentArray.push_back(std::move(component));
  }

  template <typename... Args>
  void EmplaceData(Entity entity, Args &&...args) {
    // assert(entityToIndexMap.find(entity) == entityToIndexMap.end() &&
    //        "Component added to same entity more than once.");
    std::size_t page = entity.Id() / PAGE_SIZE;
    std::size_t offset = entity.Id() % PAGE_SIZE;
    auto *p = GetPageRequired(page);
    p[offset] = static_cast<uint32_t>(dense.size());
    dense.push_back(entity);
    componentArray.emplace_back(std::forward<Args>(args)...);
  }

  // TODO : lazy removal
  void RemoveData(Entity entity) {
    std::size_t page = entity.Id() / PAGE_SIZE;
    std::size_t offset = entity.Id() % PAGE_SIZE;
    auto *p = GetPage(page);
    if (!p) return;

    uint32_t idx = p[offset];

    Entity lastEntity = dense.back();
    std::size_t lastPage = lastEntity.Id() / PAGE_SIZE;
    std::size_t lastOffset = lastEntity.Id() % PAGE_SIZE;

    dense[idx] = lastEntity;
    componentArray[idx] = std::move(componentArray.back());
    sparse[lastPage]->at(lastOffset) = idx;

    dense.pop_back();
    componentArray.pop_back();
    p[offset] = TOMBSTONE;
  }

  T &GetData(Entity entity) {
    std::size_t page = entity.Id() / PAGE_SIZE;
    std::size_t offset = entity.Id() % PAGE_SIZE;
    return componentArray[sparse[page]->at(offset)];
  }

  std::vector<Entity> GetAllEntities() override { return dense; }

  // Called when entity is destoryed
  void EntityDestroyed(Entity entity) override {
    if (HasEntity(entity)) {
      RemoveData(entity);
    }
  }

  std::size_t GetSize() const override { return componentArray.size(); }
};
