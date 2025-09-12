#ifndef COMPONENTS_TEXTCOMPONENT_
#define COMPONENTS_TEXTCOMPONENT_

#include "SDL.h"

struct TextComponent {
  char text[20];
  SDL_Color color;
  int x = 0;
  int y = 0;

  // Cache
  SDL_Texture *texture = nullptr;
  int w = 0;
  int h = 0;
  bool isDirty = true;
};

#endif /* COMPONENTS_TEXTCOMPONENT_ */
