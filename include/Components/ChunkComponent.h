#ifndef COMPONENTS_CHUNKCOMPONENT_
#define COMPONENTS_CHUNKCOMPONENT_

#include "TileData.h"
#include "SDL.h"

struct ChunkComponent {
  SDL_Texture* chunkTexture = nullptr; // Pre-rendered texture for the entire chunk
  bool needsRedraw = true;             // Flag to indicate if the chunk texture needs updating
};

#endif /* COMPONENTS_CHUNKCOMPONENT_ */
