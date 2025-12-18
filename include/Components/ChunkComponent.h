#pragma once
 
#include "SDL.h"

struct ChunkComponent {
  SDL_Texture *chunkTexture;
  bool bNeedsRedraw;
};
