#include "System/RenderSystem.h"

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
  for (EntityID entity :
       registry->view<SpriteComponent, TransformComponent>()) {
    auto& sprite = registry->getComponent<SpriteComponent>(entity);
    auto& transform = registry->getComponent<TransformComponent>(entity);

    SDL_Rect destRect = {static_cast<int>(transform.xPos),
                         static_cast<int>(transform.yPos),
                         static_cast<int>(sprite.srcRect.w * transform.xScale),
                         static_cast<int>(sprite.srcRect.h * transform.yScale)};

    // 컴포넌트의 텍스처 포인터를 사용해 렌더링
    SDL_RenderCopy(renderer, sprite.texture, &sprite.srcRect, &destRect);
    SDL_RenderPresent(renderer);
  }
}