#include "System/AssemblingMachineSystem.h"
#include "Components/AssemblingMachineComponent.h"
#include "Components/AnimationComponent.h"
#include "Components/TimerComponent.h"
#include "Core/Registry.h"
#include "Core/Recipe.h"
#include "Core/Item.h"
#include "Core/TimerManager.h"
#include "Util/TimerUtil.h"
#include <algorithm>

AssemblingMachineSystem::AssemblingMachineSystem(Registry* r, TimerManager* tm) : registry(r), timerManager(tm) {}

void AssemblingMachineSystem::Update(float deltaTime) {
  auto view = registry->view<AssemblingMachineComponent>();
  
  for (auto entity : view) {
    auto& machine = registry->GetComponent<AssemblingMachineComponent>(entity);
    
    switch (machine.state) {
      case AssemblingMachineState::Idle:
        if (machine.currentRecipe != RecipeID::None && HasEnoughIngredients(entity) && CanStoreOutput(entity)) {
          StartCrafting(entity, machine);
        }
        break;
        
      case AssemblingMachineState::WaitingForIngredients:
        if (HasEnoughIngredients(entity) && CanStoreOutput(entity)) {
          StartCrafting(entity, machine);
        }
        break;
        
      case AssemblingMachineState::Crafting:
        // Timer system handles the crafting duration
        // Check if crafting timer has expired via TimerExpiredTag
        if (registry->HasComponent<TimerExpiredTag>(entity)) {
          auto& expiredTag = registry->GetComponent<TimerExpiredTag>(entity);
          if (expiredTag.expiredId == TimerId::AssemblingMachineCraft) {
            // Crafting complete
            ProduceOutput(entity, machine);
            registry->RemoveComponent<TimerExpiredTag>(entity);
            
            // Check if we can continue crafting
            if (HasEnoughIngredients(entity) && CanStoreOutput(entity)) {
              StartCrafting(entity, machine);
            } else if (!CanStoreOutput(entity)) {
              machine.state = AssemblingMachineState::OutputFull;
              machine.isAnimating = false;
            } else {
              machine.state = AssemblingMachineState::WaitingForIngredients;
              machine.isAnimating = false;
            }
          }
        }
        break;
        
      case AssemblingMachineState::OutputFull:
        if (CanStoreOutput(entity)) {
          machine.state = AssemblingMachineState::Idle;
        }
        break;
    }
    
    UpdateAnimationState(entity, machine);
  }
}

void AssemblingMachineSystem::SetRecipe(EntityID entity, RecipeID recipeId) {
  if (!registry->HasComponent<AssemblingMachineComponent>(entity)) return;
  
  auto& machine = registry->GetComponent<AssemblingMachineComponent>(entity);
  machine.currentRecipe = recipeId;
  machine.showRecipeSelection = false;
  machine.state = AssemblingMachineState::Idle;
}

void AssemblingMachineSystem::ClearRecipe(EntityID entity) {
  if (!registry->HasComponent<AssemblingMachineComponent>(entity)) return;
  
  auto& machine = registry->GetComponent<AssemblingMachineComponent>(entity);
  machine.currentRecipe = RecipeID::None;
  machine.showRecipeSelection = true;
  machine.state = AssemblingMachineState::Idle;
  machine.isAnimating = false;
  
  // Stop any crafting timer
  util::DetachTimer(registry, timerManager, entity, TimerId::AssemblingMachineCraft);
}

bool AssemblingMachineSystem::AddInputItem(EntityID entity, ItemID itemId, int amount) {
  if (!registry->HasComponent<AssemblingMachineComponent>(entity)) return false;
  
  auto& machine = registry->GetComponent<AssemblingMachineComponent>(entity);
  const auto& itemData = ItemDatabase::instance().get(itemId);
  
  auto it = machine.inputInventory.find(itemId);
  int currentAmount = (it != machine.inputInventory.end()) ? it->second : 0;
  
  int spaceAvailable = itemData.maxStackSize - currentAmount;
  int amountToAdd = std::min(amount, spaceAvailable);
  
  if (amountToAdd > 0) {
    machine.inputInventory[itemId] += amountToAdd;
    return true;
  }
  return false;
}

int AssemblingMachineSystem::TakeOutputItem(EntityID entity, ItemID itemId, int requestedAmount) {
  if (!registry->HasComponent<AssemblingMachineComponent>(entity)) return 0;
  
  auto& machine = registry->GetComponent<AssemblingMachineComponent>(entity);
  auto it = machine.outputInventory.find(itemId);
  if (it == machine.outputInventory.end()) return 0;
  
  int availableAmount = it->second;
  int amountToTake = std::min(requestedAmount, availableAmount);
  
  it->second -= amountToTake;
  if (it->second <= 0) {
    machine.outputInventory.erase(it);
  }
  
  return amountToTake;
}

bool AssemblingMachineSystem::HasEnoughIngredients(EntityID entity) const {
  if (!registry->HasComponent<AssemblingMachineComponent>(entity)) return false;
  
  const auto& machine = registry->GetComponent<AssemblingMachineComponent>(entity);
  if (machine.currentRecipe == RecipeID::None) return false;
  
  const auto& recipeData = RecipeDatabase::instance().get(machine.currentRecipe);
  for (const auto& ingredient : recipeData.ingredients) {
    auto it = machine.inputInventory.find(ingredient.itemId);
    if (it == machine.inputInventory.end() || it->second < ingredient.amount) {
      return false;
    }
  }
  return true;
}

bool AssemblingMachineSystem::CanStoreOutput(EntityID entity) const {
  if (!registry->HasComponent<AssemblingMachineComponent>(entity)) return false;
  
  const auto& machine = registry->GetComponent<AssemblingMachineComponent>(entity);
  if (machine.currentRecipe == RecipeID::None) return false;
  
  const auto& recipeData = RecipeDatabase::instance().get(machine.currentRecipe);
  const auto& itemData = ItemDatabase::instance().get(recipeData.outputItem);
  
  auto it = machine.outputInventory.find(recipeData.outputItem);
  int currentAmount = (it != machine.outputInventory.end()) ? it->second : 0;
  
  return currentAmount + recipeData.outputAmount <= itemData.maxStackSize;
}

AssemblingMachineState AssemblingMachineSystem::GetState(EntityID entity) const {
  if (!registry->HasComponent<AssemblingMachineComponent>(entity)) return AssemblingMachineState::Idle;
  
  const auto& machine = registry->GetComponent<AssemblingMachineComponent>(entity);
  return machine.state;
}

void AssemblingMachineSystem::ShowUI(EntityID entity, bool show) {
  if (!registry->HasComponent<AssemblingMachineComponent>(entity)) return;
  
  auto& machine = registry->GetComponent<AssemblingMachineComponent>(entity);
  machine.showUI = show;
}

void AssemblingMachineSystem::ShowRecipeSelection(EntityID entity, bool show) {
  if (!registry->HasComponent<AssemblingMachineComponent>(entity)) return;
  
  auto& machine = registry->GetComponent<AssemblingMachineComponent>(entity);
  machine.showRecipeSelection = show;
}

void AssemblingMachineSystem::ConsumeIngredients(EntityID entity, AssemblingMachineComponent& machine) {
  if (machine.currentRecipe == RecipeID::None) return;
  
  const auto& recipeData = RecipeDatabase::instance().get(machine.currentRecipe);
  for (const auto& ingredient : recipeData.ingredients) {
    machine.inputInventory[ingredient.itemId] -= ingredient.amount;
    if (machine.inputInventory[ingredient.itemId] <= 0) {
      machine.inputInventory.erase(ingredient.itemId);
    }
  }
}

void AssemblingMachineSystem::ProduceOutput(EntityID entity, AssemblingMachineComponent& machine) {
  if (machine.currentRecipe == RecipeID::None) return;
  
  const auto& recipeData = RecipeDatabase::instance().get(machine.currentRecipe);
  machine.outputInventory[recipeData.outputItem] += recipeData.outputAmount;
}

void AssemblingMachineSystem::StartCrafting(EntityID entity, AssemblingMachineComponent& machine) {
  ConsumeIngredients(entity, machine);
  machine.state = AssemblingMachineState::Crafting;
  machine.isAnimating = true;
  
  // Start crafting timer using TimerUtil
  if (machine.currentRecipe != RecipeID::None) {
    const auto& recipeData = RecipeDatabase::instance().get(machine.currentRecipe);
    util::AttachTimer(registry, timerManager, entity, TimerId::AssemblingMachineCraft, recipeData.craftingTime, false);
  }
}

void AssemblingMachineSystem::UpdateAnimationState(EntityID entity, AssemblingMachineComponent& machine) {
  if (!registry->HasComponent<AnimationComponent>(entity)) return;
  
  auto& animation = registry->GetComponent<AnimationComponent>(entity);
  
  if (machine.isAnimating && animation.currentAnimationName != "working") {
    animation.currentAnimationName = "working";
    animation.currentFrameIndex = 0;
    animation.frameTimer = 0.0f;
    animation.isPlaying = true;
  } else if (!machine.isAnimating && animation.currentAnimationName != "idle") {
    animation.currentAnimationName = "idle";
    animation.currentFrameIndex = 0;
    animation.frameTimer = 0.0f;
    animation.isPlaying = false;
  } else {
    animation.isPlaying = machine.isAnimating;
  }
}