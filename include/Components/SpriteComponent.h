#pragma once

#include "SDL_render.h"
using render_order_t = unsigned int;

struct SpriteComponent {
  SDL_Texture* texture;
  SDL_Rect srcRect;
  SDL_Rect renderRect;
  SDL_RendererFlip flip;
  render_order_t renderOrder;
};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<SpriteComponent> == true);
static_assert(std::is_trivially_destructible_v<SpriteComponent> == true);
#endif