// core/ComponentArray.h
#pragma once
#include <cassert>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Entity.h"

// 1. 공통 인터페이스
class IComponentArray {
 public:
  virtual ~IComponentArray() = default;
  virtual void entityDestroyed(EntityID entity) = 0;
};

// 2. 실제 컴포넌트 데이터를 담을 템플릿 클래스
template <typename T>
class ComponentArray : public IComponentArray {
 private:
  // 컴포넌트 데이터를 연속된 메모리에 저장 (캐시 효율)
  std::vector<T> m_componentArray;

  // 엔티티 ID -> 데이터 배열의 인덱스
  std::unordered_map<EntityID, size_t> m_entityToIndexMap;
  // 데이터 배열의 인덱스 -> 엔티티 ID (빠른 삭제를 위함)
  std::unordered_map<size_t, EntityID> m_indexToEntityMap;

 public:
  void addData(EntityID entity, T component) {
    assert(m_entityToIndexMap.find(entity) == m_entityToIndexMap.end() &&
           "Component added to same entity more than once.");

    size_t newIndex = m_componentArray.size();
    m_entityToIndexMap[entity] = newIndex;
    m_indexToEntityMap[newIndex] = entity;
    m_componentArray.push_back(component);
  }

  void removeData(EntityID entity) {
    assert(m_entityToIndexMap.find(entity) != m_entityToIndexMap.end() &&
           "Removing non-existent component.");

    // 빠른 삭제 기법:
    // 1. 삭제할 요소의 인덱스를 찾는다.
    size_t indexOfRemovedEntity = m_entityToIndexMap[entity];
    // 2. 배열의 맨 마지막 요소를 삭제할 위치로 옮긴다.
    size_t indexOfLastElement = m_componentArray.size() - 1;
    m_componentArray[indexOfRemovedEntity] =
        m_componentArray[indexOfLastElement];

    // 3. 맵 정보를 업데이트한다.
    EntityID entityOfLastElement = m_indexToEntityMap[indexOfLastElement];
    m_entityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
    m_indexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

    // 4. 맨 뒤의 요소와 맵을 삭제한다.
    m_componentArray.pop_back();
    m_entityToIndexMap.erase(entity);
    m_indexToEntityMap.erase(indexOfLastElement);
  }

  template <typename... Args>
  void emplaceData(EntityID entity, Args&&... args) {
    assert(m_entityToIndexMap.find(entity) == m_entityToIndexMap.end() &&
           "Component added to same entity more than once.");
    size_t newIndex = m_componentArray.size();
    m_entityToIndexMap[entity] = newIndex;
    m_indexToEntityMap[newIndex] = entity;
    m_componentArray.push_back(T{std::forward<Args>(args)...});
  }

  T& getData(EntityID entity) {
    assert(m_entityToIndexMap.find(entity) != m_entityToIndexMap.end() &&
           "Retrieving non-existent component.");
    return m_componentArray[m_entityToIndexMap[entity]];
  }

  bool hasEntity(EntityID entity) {
    for (auto& [e, _] : m_entityToIndexMap) {
      if (e == entity) return true;
    }
    return false;
  }

  std::vector<EntityID> getAllEntity() {
    std::vector<EntityID> res;
    for (auto& [id, _] : m_entityToIndexMap) {
      res.push_back(id);
    }
    return res;
  }

  // 엔티티가 파괴될 때 호출되는 콜백
  void entityDestroyed(EntityID entity) override {
    if (m_entityToIndexMap.count(entity)) {
      removeData(entity);
    }
  }
};