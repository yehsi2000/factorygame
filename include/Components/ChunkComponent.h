#pragma once

struct SDL_Texture;

struct ChunkComponent {
  SDL_Texture *chunkTexture;
  bool isDirty;
};
