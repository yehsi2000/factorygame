#ifndef SYSTEM_INTERACTIONSYSTEM_
#define SYSTEM_INTERACTIONSYSTEM_

#include "CommandQueue.h"
#include "EventDispatcher.h"
#include "Event.h"

class InteractionSystem {
public:
    InteractionSystem(EventDispatcher* dispatcher, CommandQueue* commandQueue);
    void Update(); // In case the system needs a per-frame update in the future.

private:
    void OnInteractEvent(const InteractEvent& event);

    CommandQueue* commandQueue;
    EventHandle handle;
};

#endif /* SYSTEM_INTERACTIONSYSTEM_ */
