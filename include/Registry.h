#ifndef __REGISTRY__
#define __REGISTRY__

#include <queue>
#include <string>
#include <typeinfo>

#include "ComponentArray.h"

class Registry {
 private:
  // 엔티티 관리
  std::queue<EntityID> m_availableEntities{};
  uint32_t m_livingEntityCount = 0;

  // 컴포넌트 관리
  // 컴포넌트 타입 이름 -> 해당 타입의 ComponentArray
  std::unordered_map<const char*, std::shared_ptr<IComponentArray>>
      m_componentArrays{};

  // 컴포넌트 타입별로 유니크한 ID를 부여하는 헬퍼
  template <typename T>
  const char* getComponentTypeName() {
    return typeid(T).name();
  }

  // 컴포넌트 배열을 가져오는 내부 헬퍼
  template <typename T>
  std::shared_ptr<ComponentArray<T>> getComponentArray() {
    const char* typeName = getComponentTypeName<T>();
    assert(m_componentArrays.count(typeName) &&
           "Component type not registered before use.");
    return std::static_pointer_cast<ComponentArray<T>>(
        m_componentArrays[typeName]);
  }

 public:
  Registry() {
    // 사용 가능한 엔티티 ID 풀을 미리 생성
    for (EntityID entity = 0; entity < 5000; ++entity) {
      m_availableEntities.push(entity);
    }
  }

  // 엔티티 생성
  EntityID createEntity() {
    assert(m_livingEntityCount < 5000 && "Too many entities in existence.");
    EntityID id = m_availableEntities.front();
    m_availableEntities.pop();
    m_livingEntityCount++;
    return id;
  }

  // 엔티티 파괴
  void destroyEntity(EntityID entity) {
    assert(m_livingEntityCount > 0 && "Destroying non-existent entity.");

    // 이 엔티티에 연결된 모든 컴포넌트를 삭제
    for (auto const& pair : m_componentArrays) {
      pair.second->entityDestroyed(entity);
    }

    m_availableEntities.push(entity);
    m_livingEntityCount--;
  }

  // 컴포넌트 등록 (최초 한번만 호출)
  template <typename T>
  void registerComponent() {
    const char* typeName = getComponentTypeName<T>();
    if (m_componentArrays.find(typeName) == m_componentArrays.end()) {
      m_componentArrays[typeName] = std::make_shared<ComponentArray<T>>();
    }
  }

  // 엔티티에 컴포넌트 추가
  // 엔티티에 컴포넌트 추가 (Emplace 방식)
  template <typename T, typename... Args>
  void addComponent(EntityID entity, Args&&... args) {
    getComponentArray<T>()->emplaceData(entity, std::forward<Args>(args)...);
  }

  template <typename T>
  void removeComponent(EntityID entity) {
    getComponentArray<T>()->removeData(entity);
  }

  // 엔티티의 컴포넌트 가져오기
  template <typename T>
  T& getComponent(EntityID entity) {
    return getComponentArray<T>()->getData(entity);
  }

  // 엔티티가 특정 컴포넌트를 가지고 있는지 확인
  template <typename T>
  bool hasComponent(EntityID entity) {
    const char* typeName = getComponentTypeName<T>();
    auto it = m_componentArrays.find(typeName);
    if (it == m_componentArrays.end()) {
      return false;
    }
    // IComponentArray의 가상 함수를 통해 바로 호출
    return it->second->hasEntity(entity);
  }

  // 시스템을 위한 뷰 제공
  // 특정 컴포넌트를 가진 모든 엔티티를 순회하고 싶을 때 사용
  template <typename T>
  std::vector<EntityID> view() {
    return getComponentArray<T>()->getAllEntities();
  }

  // 모든 component에 함수 적용
  template <typename T, typename Func>
  void forEach(Func func) {
    getComponentArray<T>()->forEach(func);
  }
};

#endif