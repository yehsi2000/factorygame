#ifndef COMMANDS_RESOURCEMINECOMMAND_
#define COMMANDS_RESOURCEMINECOMMAND_

#include <iostream>

#include "Common.h"
#include "Components/InventoryComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "Components/SpriteComponent.h"
#include "Core/Command.h"
#include "Core/Entity.h"
#include "Core/Event.h"
#include "Core/GEngine.h"
#include "Core/Registry.h"

// A command to execute mining interaction with resource node
class ResourceMineCommand : public Command {
 public:
  ResourceMineCommand(EntityID instigator, EntityID target)
      : instigator(instigator), target(target) {}

  void Execute(GEngine &engine, Registry &registry) override {
    if (registry.HasComponent<ResourceNodeComponent>(target)) {
      ResourceNodeComponent &resource =
          registry.GetComponent<ResourceNodeComponent>(target);
      if (resource.LeftResource == 0) return;
      if (registry.HasComponent<InventoryComponent>(instigator)) {
        InventoryComponent &inventory =
            registry.GetComponent<InventoryComponent>(instigator);
        if (inventory.items.size() < inventory.column * inventory.row)
          resource.LeftResource--;
        engine.GetDispatcher()->Publish(ItemAddEvent(
            instigator, OreToItemMapper::instance().get(resource.Ore), 1));
      }
      if (registry.HasComponent<SpriteComponent>(target)) {
        auto &sprite = registry.GetComponent<SpriteComponent>(target);
        auto &resource = registry.GetComponent<ResourceNodeComponent>(target);
        World *world = engine.GetWorld();
        rsrc_amt_t minIron = world->GetMinironOreAmount();
        rsrc_amt_t maxIron = world->GetMaxironOreAmount();

        int richnessIndex =
            (IRON_SPRITESHEET_HEIGHT - 1) -
            std::min(
                7.0f,
                std::floor(static_cast<float>(resource.LeftResource - minIron) /
                           static_cast<float>(maxIron - minIron) * 8.f));
        sprite.srcRect = {0, richnessIndex * 128, 128, 128};
      }
    }
    std::cout << "Interaction Command Executed!" << std::endl;
  }

 private:
  EntityID instigator;
  EntityID target;
};

#endif/* COMMANDS_RESOURCEMINECOMMAND_ */
