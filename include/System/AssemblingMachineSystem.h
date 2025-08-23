#ifndef SYSTEM_ASSEMBLINGMACHINESYSTEM_
#define SYSTEM_ASSEMBLINGMACHINESYSTEM_

#include "Components/AssemblingMachineComponent.h"
#include "Core/Entity.h"
#include "Core/Item.h"
#include "Core/Recipe.h"

class Registry;
class TimerManager;

class AssemblingMachineSystem {
  Registry* registry;
  TimerManager* timerManager;

public:
  AssemblingMachineSystem(Registry* r, TimerManager* tm);
  void Update();
  
  // Recipe management
  void SetRecipe(EntityID entity, RecipeID recipeId);
  void ClearRecipe(EntityID entity);
  
  // Inventory management
  bool AddInputItem(EntityID entity, ItemID itemId, int amount);
  int TakeOutputItem(EntityID entity, ItemID itemId, int requestedAmount);
  
  // State queries
  bool HasEnoughIngredients(EntityID entity) const;
  bool CanStoreOutput(EntityID entity) const;
  AssemblingMachineState GetState(EntityID entity) const;
  
  // UI management
  void ShowUI(EntityID entity, bool show);
  void ShowRecipeSelection(EntityID entity, bool show);

private:
  void UpdateCrafting(EntityID entity, AssemblingMachineComponent& machine, float deltaTime);
  void ConsumeIngredients(EntityID entity, AssemblingMachineComponent& machine);
  void ProduceOutput(EntityID entity, AssemblingMachineComponent& machine);
  void StartCrafting(EntityID entity, AssemblingMachineComponent& machine);
  void UpdateAnimationState(EntityID entity, AssemblingMachineComponent& machine);
};

#endif/* SYSTEM_ASSEMBLINGMACHINESYSTEM_ */
