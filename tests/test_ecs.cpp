#include "Components/MovementComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Registry.h"
#include "Core/EventDispatcher.h"
#include <SDL.h>

#include <iostream>
#include <utility>

bool test_entity_creation() {
  EventDispatcher eventDispatcher;
  Registry registry(&eventDispatcher);

  // Test entity creation
  auto entity = registry.CreateEntity();
  if (entity == Entity::Null()) {
    std::cerr << "Entity creation failed" << std::endl;
    return false;
  }

  // Test component addition
  registry.EmplaceComponent<TransformComponent>(entity, Vec2f{10.0f, 20.0f});

  if (!registry.HasComponent<TransformComponent>(entity)) {
    std::cerr << "Component addition failed" << std::endl;
    return false;
  }

  return true;
}

bool test_component_access() {
  EventDispatcher eventDispatcher;
  Registry registry(&eventDispatcher);

  auto entity = registry.CreateEntity();
  Vec2f testPos{100.0f, 200.0f};

  registry.EmplaceComponent<TransformComponent>(entity, testPos);

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
  EventDispatcher eventDispatcher;
  Registry registry(&eventDispatcher);

  // Create entities with different component combinations
  auto entity1 = registry.CreateEntity();
  registry.EmplaceComponent<TransformComponent>(entity1, Vec2f{0, 0});

  auto entity2 = registry.CreateEntity();
  registry.EmplaceComponent<TransformComponent>(entity2, Vec2f{10, 10});
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

struct ViewTagA {
  int v = 1;
};
struct ViewTagB {
  int v = 2;
};

// A component type id is cached in a function-local static, so the counter
// feeding it must be process-wide. A per-instance counter lets a second
// Registry hand two different types the same id, merging their storage.
bool test_component_ids_unique_across_registries() {
  EventDispatcher eventDispatcher;
  {
    Registry first(&eventDispatcher);
    Entity e = first.CreateEntity();
    first.EmplaceComponent<TransformComponent>(e, Vec2f{0.f, 0.f});
  }

  Registry registry(&eventDispatcher);
  Entity transformOnly = registry.CreateEntity();
  registry.EmplaceComponent<TransformComponent>(transformOnly, Vec2f{0.f, 0.f});
  Entity movementOnly = registry.CreateEntity();
  registry.EmplaceComponent<MovementComponent>(movementOnly, 1.0f);

  if (registry.view<TransformComponent>().size() != 1) {
    std::cerr << "Type id collision: view<Transform> returned "
              << registry.view<TransformComponent>().size() << ", expected 1"
              << std::endl;
    return false;
  }

  if (registry.HasComponent<MovementComponent>(transformOnly) ||
      registry.HasComponent<TransformComponent>(movementOnly)) {
    std::cerr << "Type id collision: two component types share one array"
              << std::endl;
    return false;
  }

  return true;
}

// view() seeds its candidate set from the smallest array and must then filter
// against every other type -- skipping any of them returns entities that lack
// a component, and the caller's GetComponent then reads out of bounds.
bool test_view_filters_every_component() {
  EventDispatcher eventDispatcher;
  Registry registry(&eventDispatcher);

  Entity aOnly = registry.CreateEntity();
  Entity aOnly2 = registry.CreateEntity();
  Entity both = registry.CreateEntity();
  Entity bOnly = registry.CreateEntity();

  registry.EmplaceComponent<ViewTagA>(aOnly);
  registry.EmplaceComponent<ViewTagA>(aOnly2);
  registry.EmplaceComponent<ViewTagA>(both);
  registry.EmplaceComponent<ViewTagB>(both);
  registry.EmplaceComponent<ViewTagB>(bOnly);

  // |TagA| = 3 but |TagB| = 2, so candidates come from the TagB array, which
  // puts TagA at index 0 -- the slot the old prune loop never checked.
  auto ab = registry.view<ViewTagA, ViewTagB>();
  if (ab.size() != 1) {
    std::cerr << "view<ViewTagA, ViewTagB> returned " << ab.size()
              << ", expected 1" << std::endl;
    return false;
  }
  if (!registry.HasComponent<ViewTagA>(ab[0]) ||
      !registry.HasComponent<ViewTagB>(ab[0])) {
    std::cerr << "view returned an entity missing one of the components"
              << std::endl;
    return false;
  }

  // Same data with the template order reversed. Which array is smallest --
  // and therefore which index the prune loop must not skip -- depends on the
  // argument order, so both orders have to give the same answer.
  auto ba = registry.view<ViewTagB, ViewTagA>();
  if (ba.size() != 1) {
    std::cerr << "view<ViewTagB, ViewTagA> returned " << ba.size()
              << ", expected 1" << std::endl;
    return false;
  }

  return true;
}

// Destroying recycles the id with a bumped generation; the old handle must
// stop resolving, and component lookups keyed by id alone must not resurrect
// the recycled entity's data.
bool test_destroy_invalidates_stale_handles() {
  EventDispatcher eventDispatcher;
  Registry registry(&eventDispatcher);

  Entity entity = registry.CreateEntity();
  registry.EmplaceComponent<TransformComponent>(entity, Vec2f{1.f, 2.f});

  Entity stale = entity;
  registry.DestroyEntity(entity);

  if (registry.HasComponent<TransformComponent>(stale)) {
    std::cerr << "Stale handle still resolves after DestroyEntity"
              << std::endl;
    return false;
  }

  Entity recycled = registry.CreateEntity();
  if (recycled.Id() != stale.Id()) {
    std::cerr << "Destroyed entity id was not recycled" << std::endl;
    return false;
  }
  if (recycled == stale) {
    std::cerr << "Recycled entity compares equal to the stale handle; "
                 "generation was not bumped"
              << std::endl;
    return false;
  }

  return true;
}

// Removing from the middle swap-moves the last element, so every other entity
// must stay resolvable. Removing something that is not there must be a no-op,
// not an out-of-bounds write through a TOMBSTONE index.
bool test_remove_component() {
  EventDispatcher eventDispatcher;
  Registry registry(&eventDispatcher);

  Entity a = registry.CreateEntity();
  Entity b = registry.CreateEntity();
  Entity c = registry.CreateEntity();
  registry.EmplaceComponent<TransformComponent>(a, Vec2f{1.f, 1.f});
  registry.EmplaceComponent<TransformComponent>(b, Vec2f{2.f, 2.f});
  registry.EmplaceComponent<TransformComponent>(c, Vec2f{3.f, 3.f});

  registry.RemoveComponent<TransformComponent>(b);

  if (registry.HasComponent<TransformComponent>(b) ||
      !registry.HasComponent<TransformComponent>(a) ||
      !registry.HasComponent<TransformComponent>(c)) {
    std::cerr << "Removing from the middle corrupted the sparse set"
              << std::endl;
    return false;
  }
  if (registry.GetComponent<TransformComponent>(a).position.x != 1.f ||
      registry.GetComponent<TransformComponent>(c).position.x != 3.f) {
    std::cerr << "Swap-remove lost or duplicated component data" << std::endl;
    return false;
  }

  // b's slot is now TOMBSTONE but its page still exists -- exactly the case
  // that used to write through index 0xFFFFFFFF.
  registry.RemoveComponent<TransformComponent>(b);
  registry.RemoveComponent<TransformComponent>(Entity::Null());

  if (registry.view<TransformComponent>().size() != 2) {
    std::cerr << "Removing a non-existent component changed the array"
              << std::endl;
    return false;
  }

  return true;
}

// Adding the same component twice must overwrite, not push a second dense
// entry: a duplicate makes every view() of that type return the entity twice.
bool test_adding_component_twice_overwrites() {
  EventDispatcher eventDispatcher;
  Registry registry(&eventDispatcher);

  Entity entity = registry.CreateEntity();
  registry.EmplaceComponent<TransformComponent>(entity, Vec2f{1.f, 1.f});
  registry.EmplaceComponent<TransformComponent>(entity, Vec2f{9.f, 9.f});

  if (registry.view<TransformComponent>().size() != 1) {
    std::cerr << "Duplicate add produced "
              << registry.view<TransformComponent>().size()
              << " dense entries, expected 1" << std::endl;
    return false;
  }
  if (registry.GetComponent<TransformComponent>(entity).position.x != 9.f) {
    std::cerr << "Re-adding a component did not overwrite it" << std::endl;
    return false;
  }

  return true;
}

int main(int argc, char *argv[]) {
  // Every case runs: a single failure should not hide the others.
  const std::pair<const char *, bool (*)()> tests[] = {
      {"entity_creation", test_entity_creation},
      {"component_access", test_component_access},
      {"entity_view", test_entity_view},
      {"component_ids_unique_across_registries",
       test_component_ids_unique_across_registries},
      {"view_filters_every_component", test_view_filters_every_component},
      {"destroy_invalidates_stale_handles", test_destroy_invalidates_stale_handles},
      {"remove_component", test_remove_component},
      {"adding_component_twice_overwrites",
       test_adding_component_twice_overwrites},
  };

  bool all_passed = true;
  for (const auto &test : tests) {
    if (!test.second()) {
      std::cerr << "FAILED: " << test.first << std::endl;
      all_passed = false;
    }
  }

  if (all_passed) {
    std::cout << "All ECS tests passed!" << std::endl;
    return 0;
  } else {
    std::cerr << "Some ECS tests failed!" << std::endl;
    return 1;
  }
}