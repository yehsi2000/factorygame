#include "Components/MovementComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Registry.h"
#include <SDL.h>
#include <iostream>


bool test_entity_creation() {
  Registry registry;
  registry.RegisterComponent<TransformComponent>();
  registry.RegisterComponent<MovementComponent>();

  // Test entity creation
  auto entity = registry.CreateEntity();
  if (entity == INVALID_ENTITY) {
    std::cerr << "Entity creation failed" << std::endl;
    return false;
  }

  // Test component addition
  registry.AddComponent<TransformComponent>(entity, Vec2f{10.0f, 20.0f});

  if (!registry.HasComponent<TransformComponent>(entity)) {
    std::cerr << "Component addition failed" << std::endl;
    return false;
  }

  return true;
}

bool test_component_access() {
  Registry registry;
  registry.RegisterComponent<TransformComponent>();
  registry.RegisterComponent<MovementComponent>();

  auto entity = registry.CreateEntity();
  Vec2f testPos{100.0f, 200.0f};

  registry.AddComponent<TransformComponent>(entity, testPos);

  auto &transform = registry.GetComponent<TransformComponent>(entity);

  if (transform.position.x != testPos.x || transform.position.y != testPos.y) {
    std::cerr << "Component access failed" << std::endl;
    return false;
  }

  // Modify component
  transform.position.x = 300.0f;

  auto &modifiedTransform = registry.GetComponent<TransformComponent>(entity);
  if (modifiedTransform.position.x != 300.0f) {
    std::cerr << "Component modification failed" << std::endl;
    return false;
  }

  return true;
}

bool test_entity_view() {
  Registry registry;
  registry.RegisterComponent<TransformComponent>();
  registry.RegisterComponent<MovementComponent>();

  // Create entities with different component combinations
  auto entity1 = registry.CreateEntity();
  registry.AddComponent<TransformComponent>(entity1, Vec2f{0, 0});

  auto entity2 = registry.CreateEntity();
  registry.AddComponent<TransformComponent>(entity2, Vec2f{10, 10});
  registry.EmplaceComponent<MovementComponent>(entity2, 1.0f);

  auto entity3 = registry.CreateEntity();
  registry.EmplaceComponent<MovementComponent>(entity3, 2.0f);

  // Test view with single component
  auto transformView = registry.view<TransformComponent>();
  int transformCount = 0;
  for (auto entity : transformView) {
    transformCount++;
  }

  if (transformCount != 2) {
    std::cerr << "Single component view failed: expected 2, got "
              << transformCount << std::endl;
    return false;
  }

  // Test view with multiple components
  auto multiView = registry.view<TransformComponent, MovementComponent>();
  int multiCount = 0;
  for (auto entity : multiView) {
    multiCount++;
  }

  if (multiCount != 1) {
    std::cerr << "Multi component view failed: expected 1, got " << multiCount
              << std::endl;
    return false;
  }

  return true;
}

int main(int argc, char *argv[]) {
  bool all_passed = true;

  if (!test_entity_creation()) {
    all_passed = false;
  }

  if (!test_component_access()) {
    all_passed = false;
  }

  if (!test_entity_view()) {
    all_passed = false;
  }

  if (all_passed) {
    std::cout << "All ECS tests passed!" << std::endl;
    return 0;
  } else {
    std::cerr << "Some ECS tests failed!" << std::endl;
    return 1;
  }
}