#ifndef ASSETMANAGER_
#define ASSETMANAGER_

#include <memory>
#include <string>
#include <unordered_map>

#include "SDL.h"
#include "SDL_image.h"

// SDL_Texture를 위한 커스텀 삭제자
struct TextureDeleter {
  void operator()(SDL_Texture* texture) const {
    if (texture) {
      SDL_DestroyTexture(texture);
    }
  }
};

class AssetManager {
 public:
  static AssetManager& getInstance() {
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

#endif /* ASSETMANAGER_ */
