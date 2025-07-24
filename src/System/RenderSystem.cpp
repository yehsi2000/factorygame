#include "System/RenderSystem.h"

#include <algorithm>
#include <vector>

#include "Components/SpriteComponent.h"
#include "Components/TransformComponent.h"
#include "Entity.h"
#include "SDL.h"

RenderSystem::RenderSystem(Registry* r, SDL_Renderer* render) {
  registry = r;
  renderer = render;
}

void RenderSystem::Update() {
  SDL_SetRenderDrawColor(renderer, 0x66, 0x66, 0xBB, 0xFF);
  SDL_RenderClear(renderer);

  auto view = registry->view<SpriteComponent, TransformComponent>();

  for (EntityID entity : view) {
    auto& sprite = registry->GetComponent<SpriteComponent>(entity);
    auto& transform = registry->GetComponent<TransformComponent>(entity);

    SDL_Rect destRect = {
        static_cast<int>(transform.position.x),
        static_cast<int>(transform.position.y),
        static_cast<int>(sprite.srcRect.w * transform.scale.x),
        static_cast<int>(sprite.srcRect.h * transform.scale.y)};

    // 컴포넌트의 텍스처 포인터를 사용해 렌더링
    SDL_RenderCopy(renderer, sprite.texture, &sprite.srcRect, &destRect);
  }

  // 모든 엔티티를 그린 후, 화면에 한 번만 렌더링합니다.
  SDL_RenderPresent(renderer);
}