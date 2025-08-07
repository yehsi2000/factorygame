#ifndef COMPONENTS_INTERACTIONCOMPONENT_
#define COMPONENTS_INTERACTIONCOMPONENT_

#include "Core/Entity.h"
#include "Core/Type.h"

enum class InteractionType { INVALID, KEYBOARD, MOUSE };

struct InteractionComponent {
  EntityID interactor;  // if timer exist related to interaction, it should be
                        // attached to interactor
  Vec2 targetTileCoord;
  InteractionType type;
  float interactionTime;
  bool isInteracting = false;
};

#endif /* COMPONENTS_INTERACTIONCOMPONENT_ */
