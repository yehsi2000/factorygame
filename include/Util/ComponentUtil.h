#include "Components/InactiveComponent.h"
#include "Core/Entity.h"
#include "Core/Registry.h"


namespace util {
void SetEntityInactivatity(Registry& registry, Entity e, bool isInactivate) {
  if (registry.HasComponent<InactiveComponent>(e)) {
    registry.GetComponent<InactiveComponent>(e).isInactive = isInactivate;
  }
}

}  // namespace util