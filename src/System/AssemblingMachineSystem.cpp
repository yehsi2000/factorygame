#include "System/AssemblingMachineSystem.h"

#include <algorithm>

#include "Components/AnimationComponent.h"
#include "Components/AssemblingMachineComponent.h"
#include "Components/TimerComponent.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/Item.h"
#include "Core/Recipe.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"
#include "Util/AnimUtil.h"
#include "Util/TimerUtil.h"


AssemblingMachineSystem::AssemblingMachineSystem(const SystemContext &context)
    : registry(context.registry),
      eventDispatcher(context.eventDispatcher),
      timerManager(context.timerManager) {
  // Subscribe event handler function
  AddInputEventHandle = eventDispatcher->Subscribe<AssemblyAddInputEvent>(
      [this](const AssemblyAddInputEvent &event) {
        this->AddInputHandler(event);
      });
  TakeOutputEventHandle = eventDispatcher->Subscribe<AssemblyTakeOutputEvent>(
      [this](const AssemblyTakeOutputEvent &event) {
        this->TakeOutputHandler(event);
      });
  CraftOutputEventHandle = eventDispatcher->Subscribe<AssemblyCraftOutputEvent>(
      [this](const AssemblyCraftOutputEvent &event) {
        this->ProduceOutput(event.machine);
      });
};

void AssemblingMachineSystem::Update() {
  auto view = registry->view<AssemblingMachineComponent>();

  for (auto entity : view) {
    auto &machine = registry->GetComponent<AssemblingMachineComponent>(entity);

    switch (machine.state) {
      case AssemblingMachineState::Idle:
        if (machine.currentRecipe != RecipeID::None &&
            HasEnoughIngredients(entity) && CanStoreOutput(entity)) {
          StartCrafting(entity, machine);
        }
        break;

      case AssemblingMachineState::WaitingForIngredients:
        if (HasEnoughIngredients(entity) && CanStoreOutput(entity)) {
          StartCrafting(entity, machine);
        }
        break;

      case AssemblingMachineState::Crafting:
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

void AssemblingMachineSystem::AddInputHandler(
    const AssemblyAddInputEvent &event) {
  int amt = AddInputItem(event.machine, event.item, event.amount);
  eventDispatcher->Publish(ItemConsumeEvent{event.target, event.item, amt});
}

void AssemblingMachineSystem::TakeOutputHandler(
    const AssemblyTakeOutputEvent &event) {
  int amt = TakeOutputItem(event.machine, event.item, event.amount);
  eventDispatcher->Publish(ItemAddEvent{event.target, event.item, amt});
}

int AssemblingMachineSystem::AddInputItem(EntityID entity, ItemID itemId,
                                          int amount) {
  if (!registry->HasComponent<AssemblingMachineComponent>(entity)) return false;

  auto &machine = registry->GetComponent<AssemblingMachineComponent>(entity);
  const auto &itemData = ItemDatabase::instance().get(itemId);

  auto it = machine.inputInventory.find(itemId);
  int currentAmount = (it != machine.inputInventory.end()) ? it->second : 0;

  int spaceAvailable = itemData.maxStackSize - currentAmount;
  int amountToAdd = std::min(amount, spaceAvailable);

  if (amountToAdd > 0) {
    machine.inputInventory[itemId] += amountToAdd;
    return amountToAdd;
  }
  return -1;
}

int AssemblingMachineSystem::TakeOutputItem(EntityID entity, ItemID itemId,
                                            int requestedAmount) {
  if (!registry->HasComponent<AssemblingMachineComponent>(entity)) return 0;

  auto &machine = registry->GetComponent<AssemblingMachineComponent>(entity);
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

  const auto &machine =
      registry->GetComponent<AssemblingMachineComponent>(entity);
  if (machine.currentRecipe == RecipeID::None) return false;

  const auto &recipeData =
      RecipeDatabase::instance().get(machine.currentRecipe);
  for (const auto &ingredient : recipeData.ingredients) {
    auto it = machine.inputInventory.find(ingredient.itemId);
    if (it == machine.inputInventory.end() || it->second < ingredient.amount) {
      return false;
    }
  }
  return true;
}

bool AssemblingMachineSystem::CanStoreOutput(EntityID entity) const {
  if (!registry->HasComponent<AssemblingMachineComponent>(entity)) return false;

  const auto &machine =
      registry->GetComponent<AssemblingMachineComponent>(entity);
  if (machine.currentRecipe == RecipeID::None) return false;

  const auto &recipeData =
      RecipeDatabase::instance().get(machine.currentRecipe);
  const auto &itemData = ItemDatabase::instance().get(recipeData.outputItem);

  auto it = machine.outputInventory.find(recipeData.outputItem);
  int currentAmount = (it != machine.outputInventory.end()) ? it->second : 0;

  return currentAmount + recipeData.outputAmount <= itemData.maxStackSize;
}

void AssemblingMachineSystem::ConsumeIngredients(
    EntityID entity, AssemblingMachineComponent &machine) {
  if (machine.currentRecipe == RecipeID::None) return;

  const auto &recipeData =
      RecipeDatabase::instance().get(machine.currentRecipe);
  for (const auto &ingredient : recipeData.ingredients) {
    machine.inputInventory[ingredient.itemId] -= ingredient.amount;
    if (machine.inputInventory[ingredient.itemId] <= 0) {
      machine.inputInventory.erase(ingredient.itemId);
    }
  }
}

void AssemblingMachineSystem::ProduceOutput(EntityID entity) {
  auto &machine = registry->GetComponent<AssemblingMachineComponent>(entity);
  if (machine.currentRecipe == RecipeID::None ||
      machine.state != AssemblingMachineState::Crafting)
    return;

  const auto &recipeData =
      RecipeDatabase::instance().get(machine.currentRecipe);

  machine.outputInventory[recipeData.outputItem] += recipeData.outputAmount;

  if (HasEnoughIngredients(entity) && CanStoreOutput(entity)) {
    StartCrafting(entity, machine);
  } else if (!CanStoreOutput(entity)) {
    machine.state = AssemblingMachineState::OutputFull;
    machine.bIsAnimating = false;
  } else {
    machine.state = AssemblingMachineState::WaitingForIngredients;
    machine.bIsAnimating = false;
  }
}

void AssemblingMachineSystem::StartCrafting(
    EntityID entity, AssemblingMachineComponent &machine) {
  ConsumeIngredients(entity, machine);
  machine.state = AssemblingMachineState::Crafting;
  machine.bIsAnimating = true;

  // Start crafting timer using TimerUtil
  if (machine.currentRecipe != RecipeID::None) {
    const auto &recipeData =
        RecipeDatabase::instance().get(machine.currentRecipe);
    util::AttachTimer(registry, timerManager, entity,
                      TimerId::AssemblingMachineCraft, recipeData.craftingTime,
                      false);
  }
}

void AssemblingMachineSystem::UpdateAnimationState(
    EntityID entity, AssemblingMachineComponent &machine) {
  if (!registry->HasComponent<AnimationComponent>(entity)) return;

  auto &animComp = registry->GetComponent<AnimationComponent>(entity);

  if (machine.bIsAnimating &&
      animComp.currentAnimation != AnimationName::ASSEMBLING_MACHINE_WORKING) {
    util::SetAnimation(AnimationName::ASSEMBLING_MACHINE_WORKING, animComp,
                       true);
  } else if (!machine.bIsAnimating &&
             animComp.currentAnimation !=
                 AnimationName::ASSEMBLING_MACHINE_IDLE) {
    util::SetAnimation(AnimationName::ASSEMBLING_MACHINE_IDLE, animComp, false);
  } else {
    animComp.bIsPlaying = machine.bIsAnimating;
  }
}

AssemblingMachineSystem::~AssemblingMachineSystem() = default;