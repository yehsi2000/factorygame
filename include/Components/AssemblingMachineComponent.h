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

struct AssemblingMachineComponent {
  RecipeID currentRecipe;

  AssemblingMachineState state;

  std::unordered_map<ItemID, int> inputInventory;

  std::unordered_map<ItemID, int> outputInventory;

  bool isAnimating;

  bool isShowingUI;
  bool isRecipeSelected;
};

// TODO : make trivially copyable, destructable
#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
// static_assert(std::is_trivially_copyable_v<AssemblingMachineComponent> == true);
// static_assert(std::is_trivially_destructible_v<AssemblingMachineComponent> == true);
#endif