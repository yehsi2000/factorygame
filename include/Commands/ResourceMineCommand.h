#ifndef COMMANDS_RESOURCEMINECOMMAND_
#define COMMANDS_RESOURCEMINECOMMAND_

#include <iostream>

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

      resource.LeftResource--;
      if (registry.HasComponent<InventoryComponent>(instigator)) {
        InventoryComponent &inventory =
            registry.GetComponent<InventoryComponent>(instigator);
        engine.GetDispatcher()->Publish(ItemAddEvent(
            instigator, OreToItemMapper::instance().get(resource.Ore), 1));
      }
    }
    std::cout << "Interaction Command Executed!" << std::endl;
  }

 private:
  EntityID instigator;
  EntityID target;
};

#endif /* COMMANDS_RESOURCEMINECOMMAND_ */
