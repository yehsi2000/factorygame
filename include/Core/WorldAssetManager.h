#pragma once
 
#include <memory>
#include <string>
#include <unordered_map>

#include "DataStruct/TextureDeleter.h"

class Chunk;

/**
 * @brief A singleton for managing and caching world tile assets.
 * @details Provides a centralized way to load tile textures.
 * It ensures that each tile texture is loaded only once by caching it
 * on the first request. Subsequent requests for the same tile texture return
 * the cached version, improving performance and reducing memory usage.
 */

class WorldAssetManager {
  SDL_Renderer* renderer;
  std::unordered_map<std::string, std::unique_ptr<SDL_Texture, TextureDeleter>>
      textureCache;

 public:
  // TODO : refactor cache not to use string key in hash map but to use int and vector cache
  SDL_Texture* CreateChunkTexture(Chunk& chunk);
  SDL_Texture* getTexture(const std::string& path);
  explicit WorldAssetManager(SDL_Renderer* renderer);
  ~WorldAssetManager();
};
