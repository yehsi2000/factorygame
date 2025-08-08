#include "Core/AssetManager.h"

#include <iostream>
#include <memory>

#include "SDL_image.h"

SDL_Texture *AssetManager::getTexture(const std::string &path,
                                      SDL_Renderer *renderer) {
  auto it = textureCache.find(path);
  if (it != textureCache.end()) {
    return it->second.get();
  }

  SDL_Surface *surface = IMG_Load(path.c_str());
  if (!surface) {
    std::cerr << "Failed to load image: " << path
              << " | SDL_image Error: " << IMG_GetError() << std::endl;
    return nullptr;
  }

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  if (!texture) {
    std::cerr << "Failed to create texture from surface: " << SDL_GetError()
              << std::endl;
    return nullptr;
  }

  textureCache[path] = std::unique_ptr<SDL_Texture, TextureDeleter>(texture);
  return texture;
}