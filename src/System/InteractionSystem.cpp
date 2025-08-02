#include "System/InteractionSystem.h"

#include "Commands/ResourceMineCommand.h"
#include "Components/InteractionComponent.h"

InteractionSystem::InteractionSystem(Registry* registry, World* world,
                                     EventDispatcher* dispatcher,
                                     CommandQueue* commandQueue)
    : registry(registry),
      world(world),
      commandQueue(commandQueue),
      handle(dispatcher->Subscribe<InteractEvent>(
          [this](const InteractEvent& event) {
            this->OnInteractEvent(event);
          })) {}

void InteractionSystem::OnInteractEvent(const InteractEvent& event) {
  if (!registry) return;
  if (!registry->HasComponent<InteractionComponent>(event.instigator)) return;

  const auto& interactionComp =
      registry->GetComponent<InteractionComponent>(event.instigator);

  if (!world) return;
  TileData* targetTile =
      world->GetTileAtTileCoords(interactionComp.targetTileCoord);

  if (!targetTile) return;

  commandQueue->Enqueue(std::make_unique<ResourceMineCommand>(
      event.instigator, targetTile->occupyingEntity));
  // std::cout << "Interacted immediately." << std::endl;
}

void InteractionSystem::Update() {
  // This system is purely event-driven for now, but
  // Update is here for any future polling-based logic.
}