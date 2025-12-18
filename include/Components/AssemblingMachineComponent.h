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

// TODO : make trivially copyable, destructable

struct AssemblingMachineComponent {
  RecipeID currentRecipe = RecipeID::None;
  
  AssemblingMachineState state = AssemblingMachineState::Idle;
  
  std::unordered_map<ItemID, int> inputInventory;
  
  std::unordered_map<ItemID, int> outputInventory;
  
  bool bIsAnimating = false;
  
  bool bIsShowingUI = false;
  bool bRecipeSelected = false;
};
