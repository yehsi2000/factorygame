#ifndef SYSTEM_INTERACTIONSYSTEM_
#define SYSTEM_INTERACTIONSYSTEM_

#include "Core/Event.h"
#include "Core/EventDispatcher.h"

class GEngine;

class InteractionSystem {
 public:
  InteractionSystem(GEngine* engine);
  void Update();  // In case the system needs a per-frame update in the future.

 private:
  void OnInteractEvent(const PlayerInteractEvent& event);

  GEngine* engine;
  EventHandle handle;
};

#endif /* SYSTEM_INTERACTIONSYSTEM_ */
