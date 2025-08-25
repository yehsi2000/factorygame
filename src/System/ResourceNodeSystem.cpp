#include "System/ResourceNodeSystem.h"

#include "Components/TextComponent.h"

#include "Components/ResourceNodeComponent.h"
#include "Core/Registry.h"
#include "Core/World.h"

ResourceNodeSystem::ResourceNodeSystem(Registry *r, World *world)
    : registry(r), world(world) {}

void ResourceNodeSystem::Update() {
  // Show Resource Amount
  for (EntityID entity : registry->view<ResourceNodeComponent>()) {
    if (registry->HasComponent<TextComponent>(entity)) {
      const ResourceNodeComponent &resource =
          registry->GetComponent<ResourceNodeComponent>(entity);
      TextComponent &textComp = registry->GetComponent<TextComponent>(entity);
      snprintf(textComp.text, sizeof(textComp.text), "%lld",
               static_cast<unsigned long long>(resource.LeftResource));
    }
  }
}
