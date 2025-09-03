#ifndef CORE_TEXTUREDELETER_
#define CORE_TEXTUREDELETER_

#include "SDL.h"

struct TextureDeleter {
  void operator()(SDL_Texture* texture) const {
    if (texture) {
      SDL_DestroyTexture(texture);
    }
  }
};

#endif/* CORE_TEXTUREDELETER_ */
