#ifndef SYSTEM_INTERACTIONSYSTEM_
#define SYSTEM_INTERACTIONSYSTEM_

#include "Core/Event.h"
#include "Core/EventDispatcher.h"

class CommandQueue;
class Registry;
class World;

class InteractionSystem {
 public:
  InteractionSystem(Registry* registry, World* world,
                    EventDispatcher* dispatcher, CommandQueue* commandQueue);
  void Update();  // In case the system needs a per-frame update in the future.

 private:
  void OnInteractEvent(const InteractEvent& event);

  CommandQueue* commandQueue;
  Registry* registry;
  World* world;
  EventHandle handle;
};

#endif /* SYSTEM_INTERACTIONSYSTEM_ */
