#ifndef CORE_ASSETMANAGER_
#define CORE_ASSETMANAGER_

#include <memory>
#include <string>
#include <unordered_map>

#include "SDL.h"

struct TextureDeleter {
  void operator()(SDL_Texture* texture) const {
    if (texture) {
      SDL_DestroyTexture(texture);
    }
  }
};

/**
 * @brief A singleton for managing and caching game assets.
 * @details Provides a centralized way to load assets like textures. It ensures that
 *          each asset is loaded only once by caching it on the first request.
 *          Subsequent requests for the same asset return the cached version,
 *          improving performance and reducing memory usage.
 */
class AssetManager {
 public:
  static AssetManager& Instance() {
    static AssetManager instance;
    return instance;
  }

  SDL_Texture* getTexture(const std::string& path, SDL_Renderer* renderer);

 private:
  AssetManager() = default;
  ~AssetManager() = default;

  AssetManager(const AssetManager&) = delete;
  AssetManager& operator=(const AssetManager&) = delete;

  std::unordered_map<std::string, std::unique_ptr<SDL_Texture, TextureDeleter>>
      textureCache;
};

#endif /* CORE_ASSETMANAGER_ */
