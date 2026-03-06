#pragma once

struct SDL_Texture;

struct ChunkComponent {
  SDL_Texture *chunkTexture;
  bool isDirty;
};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<ChunkComponent> == true);
static_assert(std::is_trivially_destructible_v<ChunkComponent> == true);
#endif