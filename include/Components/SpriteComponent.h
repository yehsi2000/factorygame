#ifndef COMPONENTS_SPRITECOMPONENT_
#define COMPONENTS_SPRITECOMPONENT_

#include "SDL.h"

using render_order_t = unsigned int;

struct SpriteComponent{
  SDL_Texture* texture = nullptr;
  SDL_Rect srcRect;
  SDL_Rect renderRect;
  SDL_RendererFlip flip = SDL_FLIP_NONE;
  render_order_t renderOrder = 0;
};

#endif /* COMPONENTS_SPRITECOMPONENT_ */
