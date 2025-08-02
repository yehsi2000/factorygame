#ifndef COMMANDS_RESOURCEMINECOMMAND_
#define COMMANDS_RESOURCEMINECOMMAND_

#include <iostream>

#include "Core/Command.h"
#include "Core/Entity.h"
#include "Core/GEngine.h"
#include "Core/Registry.h"

// A command to execute a generic interaction.
// In a real game, this would likely take an entity ID for the instigator
// and the target of the interaction.
class ResourceMineCommand : public Command {
 public:
  ResourceMineCommand(EntityID instigator, EntityID target)
      : target(target), instigator(instigator) {}

  void Execute(GEngine& engine, Registry& registry) override {
    if (registry.HasComponent<ResourceNodeComponent>(target)) {
      ResourceNodeComponent& resource =
          registry.GetComponent<ResourceNodeComponent>(target);
      resource.LeftResource--;
      if (registry.HasComponent<InventoryComponent>(instigator)) {
        InventoryComponent& inventory =
            registry.GetComponent<InventoryComponent>(instigator);
        inventory.items[OreToItemMapper::instance().get(resource.Ore)]++;
      }
    }
    std::cout << "Interaction Command Executed!" << std::endl;
  }

 private:
  EntityID target;
  EntityID instigator;
};

#endif /* COMMANDS_RESOURCEMINECOMMAND_ */
