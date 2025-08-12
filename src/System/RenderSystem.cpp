#include "System/RenderSystem.h"

#include <algorithm>
#include <vector>

#include "Components/CameraComponent.h"
#include "Components/ChunkComponent.h"
#include "Components/InactiveComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TextComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Chunk.h"
#include "Core/Entity.h"
#include "Core/TileData.h"
#include "SDL_ttf.h"

RenderSystem::RenderSystem(Registry* r, SDL_Renderer* render, TTF_Font* f) {
  registry = r;
  renderer = render;
  font = f;
}

void RenderSystem::Update() {
  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
  SDL_RenderClear(renderer);

  // Render chunks first (background)
  RenderChunks();

  // Render other entities on top
  RenderEntities();

  // RenderTexts();
  RenderTexts();
}

void RenderSystem::RenderChunks() {
  Vec2f cameraPos = GetCameraPosition();
  int screenWidth, screenHeight;
  SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);

  // Render all chunks that have a ChunkComponent
  auto chunkView = registry->view<ChunkComponent, TransformComponent>();

  for (EntityID entity : chunkView) {
    if (registry->HasComponent<InactiveComponent>(entity)) {
      continue;
    }
    const auto& chunk = registry->GetComponent<ChunkComponent>(entity);
    const auto& transform = registry->GetComponent<TransformComponent>(entity);

    // Convert world position to screen position
    Vec2f screenPos =
        WorldToScreen(transform.position, cameraPos, screenWidth, screenHeight);

    // Cull chunks that are off-screen
    if (screenPos.x + CHUNK_WIDTH * TILE_PIXEL_WIDTH < 0 ||
        screenPos.x > screenWidth ||
        screenPos.y + CHUNK_HEIGHT * TILE_PIXEL_HEIGHT < 0 ||
        screenPos.y > screenHeight) {
      continue;
    }

    if (chunk.chunkTexture) {
      SDL_Rect destRect = {
          static_cast<int>(screenPos.x), static_cast<int>(screenPos.y),
          CHUNK_WIDTH * TILE_PIXEL_WIDTH, CHUNK_HEIGHT * TILE_PIXEL_HEIGHT};

      SDL_RenderCopy(renderer, chunk.chunkTexture, nullptr, &destRect);
    }
  }
}

void RenderSystem::RenderEntities() {
  Vec2f cameraPos = GetCameraPosition();
  int screenWidth, screenHeight;
  SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);

  // Render all regular entities with SpriteComponent
  auto view = registry->view<SpriteComponent, TransformComponent>();

  // Create a vector of entities with their render order for sorting
  std::vector<std::pair<EntityID, int>> entitiesWithOrder;

  for (EntityID entity : view) {
    if (registry->HasComponent<InactiveComponent>(entity) ||
        registry->HasComponent<ChunkComponent>(entity)) {
      continue;
    }

    const auto& sprite = registry->GetComponent<SpriteComponent>(entity);
    entitiesWithOrder.push_back({entity, sprite.renderOrder});
  }

  // Sort entities by render order (lower values rendered first)
  std::sort(
      entitiesWithOrder.begin(), entitiesWithOrder.end(),
      [](const std::pair<EntityID, int>& a, const std::pair<EntityID, int>& b) {
        return a.second < b.second;
      });

  // Render sorted entities
  for (const auto& pair : entitiesWithOrder) {
    EntityID entity = pair.first;
    const auto& sprite = registry->GetComponent<SpriteComponent>(entity);
    const auto& transform = registry->GetComponent<TransformComponent>(entity);

    // Convert world position to screen position
    Vec2f screenPos =
        WorldToScreen(transform.position, cameraPos, screenWidth, screenHeight);

    // Simple culling - skip entities that are clearly off-screen
    float entityWidth = sprite.renderRect.w * transform.scale.x;
    float entityHeight = sprite.renderRect.h * transform.scale.y;

    if (screenPos.x + entityWidth < 0 || screenPos.x > screenWidth ||
        screenPos.y + entityHeight < 0 || screenPos.y > screenHeight) {
      continue;
    }

    SDL_Rect destRect = {static_cast<int>(screenPos.x - sprite.renderRect.x),
                         static_cast<int>(screenPos.y - sprite.renderRect.y),
                         static_cast<int>(entityWidth),
                         static_cast<int>(entityHeight)};

    SDL_RenderCopyEx(renderer, sprite.texture, &sprite.srcRect, &destRect,
                     transform.rotation, nullptr, sprite.flip);
  }
}

void RenderSystem::RenderTexts() {
  Vec2f cameraPos = GetCameraPosition();
  int screenWidth, screenHeight;
  SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);

  for (EntityID entity : registry->view<TextComponent, TransformComponent>()) {
    if (registry->HasComponent<InactiveComponent>(entity)) {
      const auto& transform =
          registry->GetComponent<TransformComponent>(entity);
      Vec2f screenPos = WorldToScreen(transform.position, cameraPos,
                                      screenWidth, screenHeight);
      SDL_Surface* textSurface =
          TTF_RenderUTF8_Blended(font, "inactive", SDL_Color(255, 0, 0));
      SDL_Texture* textTexture =
          SDL_CreateTextureFromSurface(renderer, textSurface);
      SDL_FreeSurface(textSurface);

      SDL_Rect textRect;
      textRect.x = static_cast<int>(screenPos.x);
      textRect.y = static_cast<int>(screenPos.y);
      textRect.w = textSurface->w;
      textRect.h = textSurface->h;

      SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

      SDL_DestroyTexture(textTexture);
      continue;
    }
    const auto& text = registry->GetComponent<TextComponent>(entity);
    const auto& transform = registry->GetComponent<TransformComponent>(entity);
    Vec2f screenPos =
        WorldToScreen(transform.position, cameraPos, screenWidth, screenHeight);

    if (screenPos.x < 0 || screenPos.x > screenWidth || screenPos.y < 0 ||
        screenPos.y > screenHeight) {
      continue;
    }

    SDL_Surface* textSurface =
        TTF_RenderUTF8_Blended(font, text.text, text.color);
    SDL_Texture* textTexture =
        SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect;
    textRect.x = static_cast<int>(screenPos.x);
    textRect.y = static_cast<int>(screenPos.y);
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
  }
}

Vec2f RenderSystem::GetCameraPosition() const {
  // Find the first entity with a CameraComponent
  auto cameraView = registry->view<CameraComponent>();

  for (EntityID entity : cameraView) {
    const auto& camera = registry->GetComponent<CameraComponent>(entity);
    return camera.position;
  }

  // Default camera position if no camera found
  return {0.0f, 0.0f};
}

Vec2f RenderSystem::WorldToScreen(Vec2f worldPos, Vec2f cameraPos,
                                  int screenWidth, int screenHeight) const {
  return {worldPos.x - cameraPos.x + screenWidth / 2.0f,
          worldPos.y - cameraPos.y + screenHeight / 2.0f};
}