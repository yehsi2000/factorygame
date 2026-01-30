#include "System/RenderSystem.h"

#include <algorithm>
#include <cmath>
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
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/Registry.h"
#include "Core/TileData.h"
#include "Core/World.h"
#include "SDL_ttf.h"
#include "Util/CameraUtil.h"

RenderSystem::RenderSystem(const SystemContext &context, SDL_Renderer *renderer,
                           TTF_Font *font)
    : registry(context.registry),
      renderer(renderer),
      world(context.world),
      font(font) {
  entityDestroyedEventHandle =
      context.eventDispatcher->Subscribe<EntityDestroyedEvent>(
          [this](const auto &event) { this->OnEntityDestroyed(event); });
}

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

void RenderSystem::OnEntityDestroyed(const EntityDestroyedEvent &event) {
  if (registry->HasComponent<TextComponent>(event.entity)) {
    auto &text = registry->GetComponent<TextComponent>(event.entity);
    if (text.texture) {
      SDL_DestroyTexture(text.texture);
      text.texture = nullptr;
    }
  }
}

void RenderSystem::RenderChunks(Vec2f cameraPos, Vec2 screenSize, float zoom) {
  // Render all chunks that have a ChunkComponent
  auto chunkView = registry->view<ChunkComponent, TransformComponent>();

  for (Entity entity : chunkView) {
    if (registry->HasComponent<InactiveComponent>(entity)) {
      continue;
    }
    const auto &chunk = registry->GetComponent<ChunkComponent>(entity);
    const auto &transform = registry->GetComponent<TransformComponent>(entity);

    // Convert world position to screen position
    Vec2f screenPos =
        util::WorldToScreen(transform.position, cameraPos, screenSize, zoom);

    Vec2 chunkPixelSize{CHUNK_WIDTH * TILE_PIXEL_SIZE,
                        CHUNK_HEIGHT * TILE_PIXEL_SIZE};
    Vec2f chunkTextureSize{chunkPixelSize.x * zoom,
                                    chunkPixelSize.y * zoom};
    // Cull chunks that are off-screen
    if (IsOffScreen(screenPos, screenSize, chunkTextureSize)) {
      continue;
    }

    if (chunk.chunkTexture) {
      SDL_Rect destRect;
      destRect.x = static_cast<int>(floorf(screenPos.x));
      destRect.y = static_cast<int>(floorf(screenPos.y));
      destRect.w = static_cast<int>(ceilf(screenPos.x + chunkTextureSize.x)) -
                   destRect.x;
      destRect.h = static_cast<int>(ceilf(screenPos.y + chunkTextureSize.y)) -
                   destRect.y;
      SDL_RenderCopy(renderer, chunk.chunkTexture, nullptr, &destRect);
    }
  }
}

void RenderSystem::RenderEntities(Vec2f cameraPos, Vec2 screenSize,
                                  float zoom) {
  // Render all regular entities with SpriteComponent
  auto view = registry->view<SpriteComponent, TransformComponent>();

  // Create a vector of entities with their render order for sorting
  std::vector<std::pair<Entity, int>> entitiesWithOrder;

  for (Entity entity : view) {
    if (registry->HasComponent<InactiveComponent>(entity) ||
        registry->HasComponent<ChunkComponent>(entity) ||
        registry->HasComponent<BuildingPreviewComponent>(entity)) {
      continue;
    }

    const auto &sprite = registry->GetComponent<SpriteComponent>(entity);
    entitiesWithOrder.emplace_back(entity, sprite.renderOrder);
  }

  // Sort entities by render order (lower values rendered first)
  std::sort(
      entitiesWithOrder.begin(), entitiesWithOrder.end(),
      [](const std::pair<Entity, int> &a, const std::pair<Entity, int> &b) {
        return a.second < b.second;
      });

  // Render sorted entities
  for (const auto &pair : entitiesWithOrder) {
    Entity entity = pair.first;
    const auto &sprite = registry->GetComponent<SpriteComponent>(entity);
    const auto &transform = registry->GetComponent<TransformComponent>(entity);

    // Convert world position to screen position
    Vec2f screenPos =
        util::WorldToScreen(transform.position, cameraPos, screenSize, zoom);

    Vec2f entitySize = {static_cast<float>(sprite.renderRect.w) * transform.scale.x * zoom,
                        static_cast<float>(sprite.renderRect.h) * transform.scale.y * zoom};

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
                               Vec2f entitySize) const {
  return (screenPos.x + entitySize.x < 0 || screenPos.x > screenSize.x ||
          screenPos.y + entitySize.y < 0 || screenPos.y > screenSize.y);
}

void RenderSystem::RenderTexts(Vec2f cameraPos, Vec2 screenSize, float zoom) {
  for (Entity entity : registry->view<TextComponent, TransformComponent>()) {
    if (registry->HasComponent<DebugRectComponent>(entity)) continue;
    // TODO :remove render invalid text
    auto &text = registry->GetComponent<TextComponent>(entity);
    const auto &transform = registry->GetComponent<TransformComponent>(entity);
    Vec2f screenPos =
        util::WorldToScreen(transform.position, cameraPos, screenSize, zoom);

    if (IsOffScreen(screenPos, screenSize, {0.f, 0.f})) {
      // if (text.texture) {
      //   SDL_DestroyTexture(text.texture);
      //   text.texture = nullptr;
      // }
      continue;
    }

    if (registry->HasComponent<InactiveComponent>(entity)) {
// Special case for inactive entities, render "inactive" text
// This part is not cached as it's a temporary state overlay
#ifdef DEBUG_TEXT
      SDL_Surface *textSurface =
          TTF_RenderUTF8_Blended(font, "inactive", SDL_Color{255, 0, 0, 255});
      SDL_Texture *textTexture =
          SDL_CreateTextureFromSurface(renderer, textSurface);

      SDL_Rect textRect;
      textRect.x = static_cast<int>(screenPos.x);
      textRect.y = static_cast<int>(screenPos.y);
      textRect.w = static_cast<int>(textSurface->w * zoom);
      textRect.h = static_cast<int>(textSurface->h * zoom);

      SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

      SDL_FreeSurface(textSurface);
      SDL_DestroyTexture(textTexture);
#endif
      continue;
    }

    if (text.isDirty) {
      if (text.texture) {
        SDL_DestroyTexture(text.texture);
        text.texture = nullptr;
      }

      SDL_Surface *textSurface =
          TTF_RenderUTF8_Blended(font, text.text, text.color);
      if (textSurface) {
        text.texture = SDL_CreateTextureFromSurface(renderer, textSurface);
        text.w = textSurface->w;
        text.h = textSurface->h;
        SDL_FreeSurface(textSurface);
      }
      text.isDirty = false;
    }

    if (text.texture) {
      SDL_Rect textRect;
      textRect.x = static_cast<int>(screenPos.x + text.x * zoom);
      textRect.y = static_cast<int>(screenPos.y + text.y * zoom);
      textRect.w = static_cast<int>(text.w * zoom);
      textRect.h = static_cast<int>(text.h * zoom);
      SDL_RenderCopy(renderer, text.texture, nullptr, &textRect);
    }
  }
}

void RenderSystem::RenderBuildingPreviews(Vec2f cameraPos, Vec2 screenSize,
                                          float zoom) {
  // Render all building previews
  // TODO :scrollout preview placement wrong
  auto previewView =
      registry->view<BuildingPreviewComponent, TransformComponent>();

  for (Entity entity : previewView) {
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

        SDL_Rect tileRect;
        const float tilePixelSizeZoomed = TILE_PIXEL_SIZE * zoom;
        tileRect.x = static_cast<int>(floorf(screenPos.x));
        tileRect.y = static_cast<int>(floorf(screenPos.y));
        tileRect.w =
            static_cast<int>(ceilf(screenPos.x + tilePixelSizeZoomed)) -
            tileRect.x;
        tileRect.h =
            static_cast<int>(ceilf(screenPos.y + tilePixelSizeZoomed)) -
            tileRect.y;

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
  auto debugView = registry->view<DebugRectComponent, TransformComponent>();

  for (Entity entity : debugView) {
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

RenderSystem::~RenderSystem() {
  for (Entity entity : registry->view<TextComponent>()) {
    auto &text = registry->GetComponent<TextComponent>(entity);
    if (text.texture) {
      SDL_DestroyTexture(text.texture);
      text.texture = nullptr;
    }
  }
}