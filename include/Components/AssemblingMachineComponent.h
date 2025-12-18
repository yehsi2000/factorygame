#pragma once
 
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
  RecipeID currentRecipe = RecipeID::None;
  
  AssemblingMachineState state = AssemblingMachineState::Idle;
  
  std::unordered_map<ItemID, int> inputInventory;
  
  std::unordered_map<ItemID, int> outputInventory;
  
  bool bIsAnimating = false;
  
  bool bIsShowingUI = false;
  bool bIsShowingRecipeSelection = true; // true when no recipe selected
};
