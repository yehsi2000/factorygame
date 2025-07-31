#ifndef COMPONENTARRAY_
#define COMPONENTARRAY_

#include <cassert>
#include <memory>
#include <unordered_map>
#include <vector>
#include <cstddef>

#include "Entity.h"

class IComponentArray {
 public:
  virtual ~IComponentArray() = default;
  virtual void entityDestroyed(EntityID entity) = 0;
  virtual bool hasEntity(EntityID entity) = 0;
  virtual std::size_t getSize() = 0;
  virtual std::vector<EntityID> getAllEntities() = 0;
};

// TODO : 병목생길시 entt스타일 sparse map으로 리팩토링 : pagenation, tombstone, 역방향 순회
template <typename T>
class ComponentArray : public IComponentArray {
 private:
  // 컴포넌트 데이터를 연속된 메모리에 저장 (캐시 효율)
  std::vector<T> componentArray;

  // 엔티티 ID -> 데이터 배열의 인덱스
  std::unordered_map<EntityID, std::size_t> entityToIndexMap;
  // 데이터 배열의 인덱스 -> 엔티티 ID (빠른 삭제를 위함)
  std::unordered_map<std::size_t, EntityID> indexToEntityMap;

 public:
  void addData(EntityID entity, T&& component) {
    assert(entityToIndexMap.find(entity) == entityToIndexMap.end() &&
           "Component added to same entity more than once.");

    std::size_t newIndex = componentArray.size();
    entityToIndexMap[entity] = newIndex;
    indexToEntityMap[newIndex] = entity;
    componentArray.push_back(std::move(component));
  }

  void removeData(EntityID entity) {
    assert(entityToIndexMap.find(entity) != entityToIndexMap.end() &&
           "Removing non-existent component.");

    // 삭제할 요소의 인덱스 찾기
    std::size_t indexOfRemovedEntity = entityToIndexMap[entity];
    // 배열의 마지막을 삭제할 위치로 옮김
    std::size_t indexOfLastElement = componentArray.size() - 1;
    componentArray[indexOfRemovedEntity] = componentArray[indexOfLastElement];

    // 맵 정보 업데이트
    EntityID entityOfLastElement = indexToEntityMap[indexOfLastElement];
    entityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
    indexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

    // 맨 뒤의 요소와 맵 삭제
    componentArray.pop_back();
    entityToIndexMap.erase(entity);
    indexToEntityMap.erase(indexOfLastElement);
  }

  template <typename... Args>
  void emplaceData(EntityID entity, Args&&... args) {
    assert(entityToIndexMap.find(entity) == entityToIndexMap.end() &&
           "Component added to same entity more than once.");
    std::size_t newIndex = componentArray.size();
    entityToIndexMap[entity] = newIndex;
    indexToEntityMap[newIndex] = entity;
    componentArray.push_back(T{std::forward<Args>(args)...});
  }

  T& getData(EntityID entity) {
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

  std::vector<EntityID> getAllEntities() override {
    std::vector<EntityID> res;
    res.reserve(entityToIndexMap.size());
    for (auto& [id, _] : entityToIndexMap) {
      res.push_back(id);
    }
    return res;
  }

  bool hasEntity(EntityID entity) override {
    // map의 count 메서드를 사용하여 O(1) 시간 복잡도로 확인
    return entityToIndexMap.count(entity) > 0;
  }

  // 엔티티가 파괴될 때 호출되는 콜백
  void entityDestroyed(EntityID entity) override {
    if (entityToIndexMap.count(entity)) {
      removeData(entity);
    }
  }

  std::size_t getSize() override {
    return componentArray.size();
  }
};

#endif /* COMPONENTARRAY_ */
