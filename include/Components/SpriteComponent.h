#ifndef COMPONENTS_SPRITECOMPONENT_
#define COMPONENTS_SPRITECOMPONENT_

#include "SDL.h"

struct SpriteComponent{
  SDL_Texture* texture = nullptr;
  SDL_Rect srcRect;
  SDL_RendererFlip flip = SDL_FLIP_NONE;
  int renderOrder = 0;
};

#endif /* COMPONENTS_SPRITECOMPONENT_ */
