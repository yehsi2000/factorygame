#include "System/InteractionSystem.h"

#include "Commands/PerformInteractionCommand.h"

#include "SDL.h"

InteractionSystem::InteractionSystem(EventDispatcher* dispatcher,
                                     CommandQueue* commandQueue)
    : commandQueue(commandQueue),
      handle(dispatcher->Subscribe<InteractEvent>(
          [this](const InteractEvent& event) {
            this->OnInteractEvent(event);
          })) {}

void InteractionSystem::OnInteractEvent(const InteractEvent& event) {
  commandQueue->Enqueue(std::make_unique<PerformInteractionCommand>());
  std::cout << "Interacted immediately. tick:"
            << SDL_GetTicks() << std::endl;
}

void InteractionSystem::Update() {
  // This system is purely event-driven for now, but
  // Update is here for any future polling-based logic.
}