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
