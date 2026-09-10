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

  // True when `idx` is a real dense slot that currently belongs to `entity`.
  // Comparing against dense[idx] (not just the id) is what rejects stale
  // handles: a recycled id carries a bumped generation.
  bool IsLiveSlot(uint32_t idx, Entity entity) const {
    return idx != TOMBSTONE && idx < dense.size() && dense[idx] == entity;
  }

 public:
  bool HasEntity(Entity entity) const override {
    std::size_t page = entity.Id() / PAGE_SIZE;
    std::size_t offset = entity.Id() % PAGE_SIZE;
    auto *p = GetPage(page);
    if (!p) return false;
    return IsLiveSlot(p[offset], entity);
  }

  void AddData(Entity entity, T &&component) {
    std::size_t page = entity.Id() / PAGE_SIZE;
    std::size_t offset = entity.Id() % PAGE_SIZE;
    auto *p = GetPageRequired(page);

    // Re-adding overwrites the existing slot instead of pushing a second
    // dense entry. A duplicate would make every view() of this type return
    // the entity twice and leak the orphaned slot forever.
    const uint32_t existing = p[offset];
    if (IsLiveSlot(existing, entity)) {
      componentArray[existing] = std::move(component);
      return;
    }

    p[offset] = static_cast<uint32_t>(dense.size());
    dense.push_back(entity);
    componentArray.push_back(std::move(component));
  }

  template <typename... Args>
  void EmplaceData(Entity entity, Args &&...args) {
    std::size_t page = entity.Id() / PAGE_SIZE;
    std::size_t offset = entity.Id() % PAGE_SIZE;
    auto *p = GetPageRequired(page);

    const uint32_t existing = p[offset];
    if (IsLiveSlot(existing, entity)) {
      componentArray[existing] = T(std::forward<Args>(args)...);
      return;
    }

    p[offset] = static_cast<uint32_t>(dense.size());
    dense.push_back(entity);
    componentArray.emplace_back(std::forward<Args>(args)...);
  }

  void RemoveData(Entity entity) {
    std::size_t page = entity.Id() / PAGE_SIZE;
    std::size_t offset = entity.Id() % PAGE_SIZE;
    auto *p = GetPage(page);
    if (!p) return;

    uint32_t idx = p[offset];

    // Nothing to remove: never added, already removed, or a stale handle
    // whose id was recycled. Without this, idx would be TOMBSTONE and the
    // swap below would write out of bounds.
    if (!IsLiveSlot(idx, entity)) return;

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
    // Reading a component the entity does not have -- or reading through a
    // stale handle whose id was recycled -- indexes a TOMBSTONE and is an
    // out-of-bounds access. Callers must check HasComponent first.
    assert(HasEntity(entity) &&
           "GetComponent on an entity without this component, or via a stale "
           "handle. Guard with HasComponent.");
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
