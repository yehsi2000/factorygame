#include "Core/WorldAssetManager.h"

#include <iostream>
#include <memory>

#include "Core/Chunk.h"
#include "SDL_image.h"


WorldAssetManager::WorldAssetManager(SDL_Renderer *renderer)
    : renderer(renderer) {}

SDL_Texture *WorldAssetManager::CreateChunkTexture(Chunk& chunk) {
  SDL_Texture *chunkTexture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      CHUNK_WIDTH * TILE_PIXEL_SIZE, CHUNK_HEIGHT * TILE_PIXEL_SIZE);

  // Set the texture as the render target
  SDL_SetRenderTarget(renderer, chunkTexture);

  // Clear the texture with transparent color
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  // Get the tileset texture for drawing individual tiles
  SDL_Texture *tilesetTexture;

  // Draw all tiles in the chunk to the texture
  for (int y = 0; y < CHUNK_HEIGHT; ++y) {
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
      TileData *tile = chunk.GetTile(x, y);

      // Determine source rectangle based on tile type
      SDL_Rect srcRect;
      switch (tile->type) {
        case TileType::Dirt:
          tilesetTexture = getTexture("assets/img/tile/dirt.png");
          srcRect = {0, 0, 64, 64};
          break;
        case TileType::Grass:
          tilesetTexture =
              getTexture("assets/img/tile/grass.png");
          srcRect = {0, 0, 64, 64};
          break;
        case TileType::Water:
          tilesetTexture =
              getTexture("assets/img/tile/water.png");
          srcRect = {0, 0, 64, 64};
          break;
        case TileType::Stone:
          tilesetTexture =
              getTexture("assets/img/tile/stone.png");
          srcRect = {0, 0, 64, 64};
          break;
        default:
          break;
      }

      // Destination rectangle for this tile in the chunk texture
      SDL_Rect destRect = {x * TILE_PIXEL_SIZE, y * TILE_PIXEL_SIZE,
                           TILE_PIXEL_SIZE, TILE_PIXEL_SIZE};
      // Render the tile to the chunk texture
      SDL_RenderCopy(renderer, tilesetTexture, &srcRect, &destRect);
    }
  }

  // Reset render target to default
  SDL_SetRenderTarget(renderer, nullptr);

  return chunkTexture;
}

SDL_Texture *WorldAssetManager::getTexture(const std::string &path) {
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

WorldAssetManager::~WorldAssetManager() = default;