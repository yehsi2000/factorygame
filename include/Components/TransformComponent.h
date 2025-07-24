#ifndef COMPONENTS_TRANSFORMCOMPONENT_
#define COMPONENTS_TRANSFORMCOMPONENT_

#include "Type.h"

struct TransformComponent {
  Vec2f position;
  Vec2f scale = {1.f,1.f};
};


#endif /* COMPONENTS_TRANSFORMCOMPONENT_ */
