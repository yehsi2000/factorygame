#pragma once

#include <unordered_map>

#include "Core/Item.h"
#include "Core/Recipe.h"


enum class AssemblingMachineState {
  Idle,
  WaitingForIngredients,
  Crafting,
  OutputFull
};

// TODO : make trivially copyable, destructable

struct AssemblingMachineComponent {
  RecipeID currentRecipe;

  AssemblingMachineState state;

  std::unordered_map<ItemID, int> inputInventory;

  std::unordered_map<ItemID, int> outputInventory;

  bool isAnimating;

  bool isShowingUI;
  bool isRecipeSelected;
};
