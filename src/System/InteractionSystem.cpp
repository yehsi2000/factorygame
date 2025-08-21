#include "System/InteractionSystem.h"

#include "Commands/AssemblingMachineInteractCommand.h"
#include "Commands/ResourceMineCommand.h"
#include "Components/AssemblingMachineComponent.h"
#include "Components/InteractionComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "Core/CommandQueue.h"
#include "Core/GEngine.h"
#include "Core/Registry.h"
#include "Core/World.h"
#include "Util/TimerUtil.h"
#include <Components/PlayerStateComponent.h>

InteractionSystem::InteractionSystem(GEngine* engine)
    : engine(engine),
      handle(engine->GetDispatcher()->Subscribe<PlayerInteractEvent>(
          [this](const PlayerInteractEvent& event) {
            this->OnInteractEvent(event);
          })) {}

void InteractionSystem::OnInteractEvent(const PlayerInteractEvent& event) {
  Registry* registry = engine->GetRegistry();
  World* world = engine->GetWorld();

  if (!registry || !world) return;

  EntityID player = engine->GetPlayer();
  EntityID targetEntity = event.target;

  if (targetEntity == INVALID_ENTITY) return;

  if (registry->HasComponent<ResourceNodeComponent>(targetEntity)) {
    if(!registry->HasComponent<PlayerStateComponent>(player)) return;

    auto& state = registry->GetComponent<PlayerStateComponent>(player);
    state.isMining = true;
    state.interactingEntity = targetEntity;

    util::AttachTimer(registry, engine->GetTimerManager(), player,
                      TimerId::Mine, 1.0f, true);
    
  } else if (registry->HasComponent<AssemblingMachineComponent>(targetEntity)) {
    engine->GetCommandQueue()->Enqueue(
        std::make_unique<AssemblingMachineInteractCommand>(player,
                                                           targetEntity));
  }
}

void InteractionSystem::Update() {
}