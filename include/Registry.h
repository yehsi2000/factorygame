#ifndef REGISTRY_
#define REGISTRY_

#include <algorithm>
#include <queue>
#include <string>
#include <typeinfo>
#include <iostream>

#include "ComponentArray.h"
#include "InputState.h"

constexpr int MAX_ENTITIES = 100000000;

class Registry {
 private:
  // 엔티티 관리
  std::queue<EntityID> availableEntities{};
  InputState inputState;

  uint32_t livingEntityCount = 0;

  // 컴포넌트 관리
  // 컴포넌트 타입 이름 -> 해당 타입의 ComponentArray
  std::unordered_map<const char*, std::shared_ptr<IComponentArray>>
      componentArrays{};

  // 컴포넌트 타입별로 유니크한 ID를 부여하는 헬퍼
  template <typename T>
  const char* GetComponentTypeName() {
    return typeid(T).name();
  }

  // 컴포넌트 배열을 가져오는 내부 헬퍼
  template <typename T>
  std::shared_ptr<ComponentArray<T>> GetComponentArray() {
    const char* typeName = GetComponentTypeName<T>();
    assert(componentArrays.count(typeName) &&
           "Component type not registered before use.");
    return std::static_pointer_cast<ComponentArray<T>>(
        componentArrays[typeName]);
  }

  // 컴포넌트 배열의 크기를 가져오는 헬퍼
  template <typename T>
  std::size_t GetComponentSize() {
    const char* typeName = GetComponentTypeName<T>();
    if (componentArrays.count(typeName) == 0) return 0;
    return componentArrays.at(typeName)->getSize();
  }

 public:
  Registry() {
    // 사용 가능한 엔티티 ID 풀을 미리 생성
    for (EntityID entity = 0; entity < MAX_ENTITIES; ++entity) {
      availableEntities.push(entity);
    }
  }

  // 엔티티 생성
  EntityID CreateEntity() {
    assert(livingEntityCount < MAX_ENTITIES && "Too many entities in existence.");
    EntityID id = availableEntities.front();
    availableEntities.pop();
    livingEntityCount++;
    //std::cout<<"Created entity with ID:" << id << std::endl;
    return id;
  }

  // 엔티티 파괴
  void DestroyEntity(EntityID entity) {
    assert(livingEntityCount > 0 && "Destroying non-existent entity.");

    // 이 엔티티에 연결된 모든 컴포넌트를 삭제
    for (auto const& pair : componentArrays) {
      pair.second->entityDestroyed(entity);
    }

    availableEntities.push(entity);
    livingEntityCount--;
  }

  // 컴포넌트 등록 (최초 한번만 호출)
  template <typename T>
  void RegisterComponent() {
    const char* typeName = GetComponentTypeName<T>();
    if (componentArrays.find(typeName) == componentArrays.end()) {
      componentArrays[typeName] = std::make_shared<ComponentArray<T>>();
    }
  }

  // 엔티티에 컴포넌트 추가
  template <typename T>
  void AddComponent(EntityID entity, T&& component) {
    GetComponentArray<T>()->addData(entity, std::move(component));
  }

  // 엔티티에 컴포넌트 추가 (Emplace 방식)
  template <typename T, typename... Args>
  void EmplaceComponent(EntityID entity, Args&&... args) {
    GetComponentArray<T>()->emplaceData(entity, std::forward<Args>(args)...);
  }

  template <typename T>
  void RemoveComponent(EntityID entity) {
    GetComponentArray<T>()->removeData(entity);
  }

  // 엔티티의 컴포넌트 가져오기
  template <typename T>
  T& GetComponent(EntityID entity) {
    return GetComponentArray<T>()->getData(entity);
  }

  // 엔티티가 특정 컴포넌트를 가지고 있는지 확인
  template <typename T>
  bool HasComponent(EntityID entity) {
    const char* typeName = GetComponentTypeName<T>();
    auto it = componentArrays.find(typeName);
    if (it == componentArrays.end()) {
      return false;
    }
    // IComponentArray의 가상 함수를 통해 바로 호출
    return it->second->hasEntity(entity);
  }

  // 시스템을 위한 뷰 제공
  // 특정 컴포넌트를 가진 모든 엔티티를 순회하고 싶을 때 사용
  template <typename... TComponent>
  std::vector<EntityID> view() {
    // 요청된 컴포넌트가 없으면 빈 벡터를 반환합니다.
    if constexpr (sizeof...(TComponent) == 0) {
      return {};
    }

    // 1. 모든 관련 컴포넌트 배열을 가져옵니다.
    std::vector<std::shared_ptr<IComponentArray>> arrays;
    // C++17 fold expression을 사용하여 배열을 채웁니다.
    (arrays.push_back(GetComponentArray<TComponent>()), ...);

    // 2. 크기순으로 정렬하여 가장 작은 배열을 찾습니다. (성능 최적화)
    std::sort(arrays.begin(), arrays.end(), [](const auto& a, const auto& b) {
      return a->getSize() < b->getSize();
    });

    // 3. 가장 작은 컴포넌트 배열의 엔티티 목록으로 결과 집합을 초기화합니다.
    std::vector<EntityID> result = arrays[0]->getAllEntities();

    // 4. 나머지 컴포넌트 배열들을 순회하며 결과 집합을 필터링합니다.
    for (size_t i = 1; i < arrays.size(); ++i) {
      result.erase(std::remove_if(result.begin(), result.end(),
                                  [&](EntityID entity) {
                                    // 이 엔티티가 현재 검사하는 컴포넌트 배열에
                                    // 없으면 제거 대상입니다.
                                    return !arrays[i]->hasEntity(entity);
                                  }),
                   result.end());
    }

    return result;
  }

  // 모든 component에 함수 적용
  template <typename T, typename Func>
  void forEach(Func func) {
    GetComponentArray<T>()->forEach(func);
  }

  inline InputState& GetInputState() { return inputState; }
};

#endif /* REGISTRY_ */
