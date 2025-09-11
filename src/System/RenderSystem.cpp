#include "System/RenderSystem.h"

#include <algorithm>
#include <vector>

#include "Components/BuildingPreviewComponent.h"
#include "Components/ChunkComponent.h"
#include "Components/DebugRectComponent.h"
#include "Components/InactiveComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TextComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Chunk.h"
#include "Core/Entity.h"
#include "Core/Registry.h"
#include "Core/TileData.h"
#include "Core/World.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "Util/CameraUtil.h"


RenderSystem::RenderSystem(const SystemContext &context, SDL_Renderer* renderer, TTF_Font *font)
    : registry(context.registry), renderer(renderer), world(context.world), font(font) {}

void RenderSystem::Update() {
  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
  SDL_RenderClear(renderer);

  Vec2f cameraPos = util::GetCameraPosition(registry);
  float zoom = util::GetCameraZoom(registry);
  int screenWidth, screenHeight;
  SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);
  Vec2 screenSize{screenWidth, screenHeight};

  // Render chunks first (background)
  RenderChunks(cameraPos, screenSize, zoom);

  // Render other entities on top
  RenderEntities(cameraPos, screenSize, zoom);

  // Render building previews on top of entities
  RenderBuildingPreviews(cameraPos, screenSize, zoom);

  RenderTexts(cameraPos, screenSize, zoom);

  RenderDebugRect(cameraPos, screenSize, zoom);
}

void RenderSystem::RenderChunks(Vec2f cameraPos, Vec2 screenSize, float zoom) {
  // Render all chunks that have a ChunkComponent
  auto chunkView = registry->view<ChunkComponent, TransformComponent>();

  for (EntityID entity : chunkView) {
    if (registry->HasComponent<InactiveComponent>(entity)) {
      continue;
    }
    const auto &chunk = registry->GetComponent<ChunkComponent>(entity);
    const auto &transform = registry->GetComponent<TransformComponent>(entity);

    // Convert world position to screen position
    Vec2f screenPos =
        util::WorldToScreen(transform.position, cameraPos, screenSize, zoom);

    Vec2 chunkTextureSize = Vec2{CHUNK_WIDTH * TILE_PIXEL_SIZE * zoom,
                                 CHUNK_HEIGHT * TILE_PIXEL_SIZE * zoom};
    // Cull chunks that are off-screen
    if (IsOffScreen(screenPos, screenSize, chunkTextureSize)) {
      continue;
    }

    if (chunk.chunkTexture) {
      SDL_Rect destRect = {static_cast<int>(screenPos.x),
                           static_cast<int>(screenPos.y), chunkTextureSize.x,
                           chunkTextureSize.y};
      SDL_RenderCopy(renderer, chunk.chunkTexture, nullptr, &destRect);
    }
  }
}

void RenderSystem::RenderEntities(Vec2f cameraPos, Vec2 screenSize,
                                  float zoom) {
  // Render all regular entities with SpriteComponent
  auto view = registry->view<SpriteComponent, TransformComponent>();

  // Create a vector of entities with their render order for sorting
  std::vector<std::pair<EntityID, int>> entitiesWithOrder;

  for (EntityID entity : view) {
    if (registry->HasComponent<InactiveComponent>(entity) ||
        registry->HasComponent<ChunkComponent>(entity) ||
        registry->HasComponent<BuildingPreviewComponent>(entity)) {
      continue;
    }

    const auto &sprite = registry->GetComponent<SpriteComponent>(entity);
    entitiesWithOrder.push_back({entity, sprite.renderOrder});
  }

  // Sort entities by render order (lower values rendered first)
  std::sort(
      entitiesWithOrder.begin(), entitiesWithOrder.end(),
      [](const std::pair<EntityID, int> &a, const std::pair<EntityID, int> &b) {
        return a.second < b.second;
      });

  // Render sorted entities
  for (const auto &pair : entitiesWithOrder) {
    EntityID entity = pair.first;
    const auto &sprite = registry->GetComponent<SpriteComponent>(entity);
    const auto &transform = registry->GetComponent<TransformComponent>(entity);

    // Convert world position to screen position
    Vec2f screenPos =
        util::WorldToScreen(transform.position, cameraPos, screenSize, zoom);

    Vec2f entitySize = {sprite.renderRect.w * transform.scale.x * zoom,
                        sprite.renderRect.h * transform.scale.y * zoom};

    // Simple culling - skip entities that are clearly off-screen
    if (IsOffScreen(screenPos, screenSize, entitySize)) {
      continue;
    }

    SDL_Rect destRect = {
        static_cast<int>(screenPos.x + sprite.renderRect.x * zoom),
        static_cast<int>(screenPos.y + sprite.renderRect.y * zoom),
        static_cast<int>(entitySize.x), static_cast<int>(entitySize.y)};
    SDL_RenderCopyEx(renderer, sprite.texture, &sprite.srcRect, &destRect,
                     transform.rotation, nullptr, sprite.flip);
  }
}

bool RenderSystem::IsOffScreen(Vec2f screenPos, Vec2 screenSize,
                               Vec2f entitySize) {
  return (screenPos.x + entitySize.x < 0 || screenPos.x > screenSize.x ||
          screenPos.y + entitySize.y < 0 || screenPos.y > screenSize.y);
}

void RenderSystem::RenderTexts(Vec2f cameraPos, Vec2 screenSize, float zoom) {
  for (EntityID entity : registry->view<TextComponent, TransformComponent>()) {
    if (registry->HasComponent<DebugRectComponent>(entity)) continue;
    if (registry->HasComponent<InactiveComponent>(entity)) {
      const auto &transform =
          registry->GetComponent<TransformComponent>(entity);
      Vec2f screenPos =
          util::WorldToScreen(transform.position, cameraPos, screenSize, zoom);
      SDL_Surface *textSurface =
          TTF_RenderUTF8_Blended(font, "inactive", SDL_Color{255, 0, 0, 255});
      SDL_Texture *textTexture =
          SDL_CreateTextureFromSurface(renderer, textSurface);
      SDL_FreeSurface(textSurface);

      SDL_Rect textRect;
      textRect.x = static_cast<int>(screenPos.x);
      textRect.y = static_cast<int>(screenPos.y);
      textRect.w = static_cast<int>(textSurface->w * zoom);
      textRect.h = static_cast<int>(textSurface->h * zoom);

      SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

      SDL_DestroyTexture(textTexture);
      continue;
    }
    const auto &text = registry->GetComponent<TextComponent>(entity);
    const auto &transform = registry->GetComponent<TransformComponent>(entity);
    Vec2f screenPos =
        util::WorldToScreen(transform.position, cameraPos, screenSize, zoom);

    if (IsOffScreen(screenPos, screenSize, {0.f, 0.f})) {
      continue;
    }

    SDL_Surface *textSurface =
        TTF_RenderUTF8_Blended(font, text.text, text.color);
    SDL_Texture *textTexture =
        SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_Rect textRect;
    textRect.x = static_cast<int>(screenPos.x + text.x * zoom);
    textRect.y = static_cast<int>(screenPos.y + text.y * zoom);
    textRect.w = static_cast<int>(textSurface->w * zoom);
    textRect.h = static_cast<int>(textSurface->h * zoom);
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
  }
}

void RenderSystem::RenderBuildingPreviews(Vec2f cameraPos, Vec2 screenSize,
                                          float zoom) {
  // Render all building previews
  auto previewView =
      registry->view<BuildingPreviewComponent, TransformComponent>();

  for (EntityID entity : previewView) {
    if (registry->HasComponent<InactiveComponent>(entity)) {
      continue;
    }

    const auto &preview =
        registry->GetComponent<BuildingPreviewComponent>(entity);
    const auto &transform = registry->GetComponent<TransformComponent>(entity);

    Vec2 tileindex = world->GetTileIndexFromWorldPosition(transform.position);
    // Render colored tile backgrounds
    for (int dy = 0; dy < preview.height; dy++) {
      for (int dx = 0; dx < preview.width; dx++) {
        Vec2f tileWorldPos = {transform.position.x + dx * TILE_PIXEL_SIZE,
                              transform.position.y + dy * TILE_PIXEL_SIZE};

        Vec2f screenPos =
            util::WorldToScreen(tileWorldPos, cameraPos, screenSize, zoom);

        SDL_Rect tileRect = {static_cast<int>(screenPos.x * zoom),
                             static_cast<int>(screenPos.y * zoom),
                             static_cast<int>(TILE_PIXEL_SIZE * zoom),
                             static_cast<int>(TILE_PIXEL_SIZE * zoom)};

        // Set color based on validity - use more visible alpha values
        if (world->HasNoOcuupyingEntity(tileindex + Vec2{dx, dy}, 1, 1)) {
          SDL_SetRenderDrawColor(renderer, 0, 255, 0,
                                 80);  // Green with transparency
        } else {
          SDL_SetRenderDrawColor(renderer, 255, 0, 0,
                                 80);  // Red with transparency
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &tileRect);
      }
    }

    // Render the building sprite if available and placement is valid
    if (registry->HasComponent<SpriteComponent>(entity)) {
      const auto &sprite = registry->GetComponent<SpriteComponent>(entity);

      Vec2f screenPos =
          util::WorldToScreen(transform.position, cameraPos, screenSize, zoom);

      SDL_Rect destRect = {
          static_cast<int>(sprite.renderRect.x * zoom + screenPos.x),
          static_cast<int>(sprite.renderRect.y * zoom + screenPos.y),
          static_cast<int>(sprite.renderRect.w * zoom),
          static_cast<int>(sprite.renderRect.h * zoom)};

      // Render with transparency
      SDL_SetTextureAlphaMod(sprite.texture, 128);  // 50% transparency
      SDL_RenderCopyEx(renderer, sprite.texture, &sprite.srcRect, &destRect,
                       transform.rotation, nullptr, sprite.flip);
      SDL_SetTextureAlphaMod(sprite.texture, 255);  // Reset to full opacity
    }
  }

  // Reset render state
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void RenderSystem::RenderDebugRect(Vec2f cameraPos, Vec2 screenSize,
                                   float zoom) {
  // Render all building previews
  auto debugView = registry->view<DebugRectComponent, TransformComponent>();

  for (EntityID entity : debugView) {
    if (registry->HasComponent<InactiveComponent>(entity)) {
      continue;
    }

    const auto &debug = registry->GetComponent<DebugRectComponent>(entity);
    const auto &transform = registry->GetComponent<TransformComponent>(entity);

    Vec2f screenPos =
        util::WorldToScreen(transform.position, cameraPos, screenSize, zoom);

    SDL_Rect tileRect = {static_cast<int>(screenPos.x + debug.offsetX * zoom),
                         static_cast<int>(screenPos.y + debug.offsetY * zoom),
                         static_cast<int>(debug.width * zoom),
                         static_cast<int>(debug.height * zoom)};
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, debug.r, debug.g, debug.b, debug.a);
    SDL_RenderDrawRect(renderer, &tileRect);
  }
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

RenderSystem::~RenderSystem() = default;