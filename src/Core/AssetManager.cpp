#include "Core/AssetManager.h"

#include <iostream>
#include <memory>

#include "SDL.h"
#include "SDL_image.h"

SDL_Texture* AssetManager::getTexture(const std::string& path,
                                      SDL_Renderer* renderer) {
  // 1. 캐시에 해당 경로의 텍스처가 있는지 확인
  auto it = textureCache.find(path);
  if (it != textureCache.end()) {
    // 2. 있다면 캐시된 텍스처의 포인터를 반환
    return it->second.get();
  }

  // 3. 캐시에 없다면 새로 로드
  SDL_Surface* surface = IMG_Load(path.c_str());
  if (!surface) {
    std::cerr << "Failed to load image: " << path
              << " | SDL_image Error: " << IMG_GetError() << std::endl;
    return nullptr;
  }

  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  if (!texture) {
    std::cerr << "Failed to create texture from surface: " << SDL_GetError()
              << std::endl;
    return nullptr;
  }

  // 4. 새로 만든 텍스처를 캐시에 저장하고 포인터를 반환
  textureCache[path] = std::unique_ptr<SDL_Texture, TextureDeleter>(texture);
  return texture;
}