#ifndef CORE_ASSETMANAGER_
#define CORE_ASSETMANAGER_

#include <memory>
#include <string>
#include <unordered_map>

#include "SDL.h"
#include "Core/TextureDeleter.h"

/**
 * @brief A singleton for managing and caching in-game entity assets.
 * @details Provides a centralized way to load entity assets like textures.
 * It ensures that each entity asset is loaded only once by caching it
 * on the first request. Subsequent requests for the same entity asset return
 * the cached version, improving performance and reducing memory usage.
 */
class AssetManager {
  SDL_Renderer* renderer;
  std::unordered_map<std::string, std::unique_ptr<SDL_Texture, TextureDeleter>>
      textureCache;

 public:
  // TODO : refactor cache not to use string key in hash map but to use int and vector cache
  SDL_Texture* getTexture(const std::string& path);
  AssetManager(SDL_Renderer* renderer);
  ~AssetManager();
};

#endif /* CORE_ASSETMANAGER_ */
