#pragma once
 
#include "SDL.h"

struct ChunkComponent {
  SDL_Texture *chunkTexture =
      nullptr; // Pre-rendered texture for the entire chunk
  bool bNeedsRedraw =
      true; // Flag to indicate if the chunk texture needs updating
};
