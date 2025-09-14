#ifndef CORE_TEXTUREDELETER_
#define CORE_TEXTUREDELETER_

#include "SDL.h"

/**
 * @brief A custom deleter for SDL_Texture smart pointers.
 * @details This functor is designed to be used with std::unique_ptr to ensure
 * that SDL_DestroyTexture is called automatically when the smart pointer goes
 * out of scope. This helps prevent resource leaks by simplifying texture
 * memory management.
 */
struct TextureDeleter {
  void operator()(SDL_Texture* texture) const {
    if (texture) {
      SDL_DestroyTexture(texture);
    }
  }
};

#endif/* CORE_TEXTUREDELETER_ */
