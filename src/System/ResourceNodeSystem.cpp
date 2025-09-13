#include "System/ResourceNodeSystem.h"

#include "Components/ResourceNodeComponent.h"
#include "Components/TextComponent.h"
#include "Core/Registry.h"
#include "Core/World.h"

ResourceNodeSystem::ResourceNodeSystem(const SystemContext &context)
    : registry(context.registry), world(context.world) {}

void ResourceNodeSystem::Update() {
  // Show Resource Amount
  for (EntityID entity : registry->view<ResourceNodeComponent>()) {
    if (registry->HasComponent<TextComponent>(entity)) {
      const auto &resource =
          registry->GetComponent<ResourceNodeComponent>(entity);
      auto &textComp = registry->GetComponent<TextComponent>(entity);
      if (strtoll(textComp.text, NULL, 10) != resource.LeftResource) {
        snprintf(textComp.text, sizeof(textComp.text), "%lld",
                 static_cast<unsigned long long>(resource.LeftResource));
        textComp.isDirty = true;
      }
    }
  }
}

ResourceNodeSystem::~ResourceNodeSystem() = default;
