#ifndef COMPONENTS_ASSEMBLINGMACHINECOMPONENT_
#define COMPONENTS_ASSEMBLINGMACHINECOMPONENT_

#include <unordered_map>
#include "Core/Recipe.h"
#include "Core/Item.h"

enum class AssemblingMachineState {
  Idle,
  WaitingForIngredients,
  Crafting,
  OutputFull
};

struct AssemblingMachineComponent {
  // Current recipe being processed
  RecipeID currentRecipe = RecipeID::None;
  
  // Current state of the machine
  AssemblingMachineState state = AssemblingMachineState::Idle;
  
  // Input inventory - maps ItemID to amount
  std::unordered_map<ItemID, int> inputInventory;
  
  // Output inventory - maps ItemID to amount  
  std::unordered_map<ItemID, int> outputInventory;
  
  // Animation control
  bool isAnimating = false;
  
  // UI state
  bool showUI = false;
  bool showRecipeSelection = true; // true when no recipe selected
};

#endif/* COMPONENTS_ASSEMBLINGMACHINECOMPONENT_ */
