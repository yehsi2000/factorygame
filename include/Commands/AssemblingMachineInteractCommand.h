#ifndef COMMANDS_ASSEMBLINGMACHINEINTERACTCOMMAND_
#define COMMANDS_ASSEMBLINGMACHINEINTERACTCOMMAND_

#include "Components/AssemblingMachineComponent.h"
#include "Core/Command.h"
#include "Core/Entity.h"
#include "Core/GEngine.h"
#include "Core/Registry.h"
#include "System/AssemblingMachineSystem.h"

// A command to open assembling machine UI when interacted
class AssemblingMachineInteractCommand : public Command {
public:
  AssemblingMachineInteractCommand(EntityID instigator, EntityID target)
      : instigator(instigator), target(target) {}

  void Execute(GEngine& engine, Registry& registry) override {
    if (!registry.HasComponent<AssemblingMachineComponent>(target)) return;
    
    // Get the assembling machine system from the engine
    // For now, we'll just show the UI directly
    auto& machine = registry.GetComponent<AssemblingMachineComponent>(target);
    machine.showUI = true;
    
    // If no recipe is selected, show recipe selection
    if (machine.currentRecipe == RecipeID::None) {
      machine.showRecipeSelection = true;
    } else {
      machine.showRecipeSelection = false;
    }
  }

private:
  EntityID instigator;
  EntityID target;
};

#endif/* COMMANDS_ASSEMBLINGMACHINEINTERACTCOMMAND_ */
