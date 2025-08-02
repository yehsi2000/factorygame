#include "System/ResourceNodeSystem.h"

#include <Components/TextComponent.h>

#include <format>

#include "Components/ResourceNodeComponent.h"
#include "Core/Item.h"
#include "Core/Registry.h"

ResourceNodeSystem::ResourceNodeSystem(Registry* r) { registry = r; }

void ResourceNodeSystem::Update() {
  for (EntityID entity : registry->view<ResourceNodeComponent>()) {
    if (registry->HasComponent<TextComponent>(entity)) {
      const ResourceNodeComponent& resource =
          registry->GetComponent<ResourceNodeComponent>(entity);
      TextComponent& textComp = registry->GetComponent<TextComponent>(entity);
      snprintf(textComp.text, sizeof(textComp.text), "%lld",
               static_cast<unsigned long long>(resource.LeftResource));
    }
  }
}
