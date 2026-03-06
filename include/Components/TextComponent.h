#pragma once

#include "SDL_pixels.h"

struct SDL_Texture;

struct TextComponent {
  char text[20];
  SDL_Color color;
  int x;
  int y;

  // Cache
  SDL_Texture *texture;
  int w;
  int h;
  bool isDirty;
};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<TextComponent> == true);
static_assert(std::is_trivially_destructible_v<TextComponent> == true);
#endif