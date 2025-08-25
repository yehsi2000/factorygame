#include "System/AnimationSystem.h"

#include "Components/AnimationComponent.h"
#include "Components/InactiveComponent.h"
#include "Components/SpriteComponent.h"
#include "Core/Entity.h"
#include "Core/Registry.h"
#include "SDL_render.h"

AnimationSystem::AnimationSystem(Registry *registry, SDL_Renderer *renderer)
    : registry(registry), renderer(renderer) {}

void AnimationSystem::Update(float deltaTime) {
  for (EntityID entity :
       registry->view<AnimationComponent, SpriteComponent>()) {
    if (registry->HasComponent<InactiveComponent>(entity)) {
      continue;
    }
    auto &anim = registry->GetComponent<AnimationComponent>(entity);
    auto &sprite = registry->GetComponent<SpriteComponent>(entity);

    if (!anim.isPlaying) {
      continue;
    }

    const auto &sequence = anim.animations.at(anim.currentAnimation);
    anim.frameTimer += deltaTime;
    // time for single frame = 1/frameRate
    if (anim.frameTimer >= (1.f / sequence.frameRate)) {
      anim.frameTimer = 0.f;
      anim.currentFrameIndex++;
      if (anim.currentFrameIndex >= sequence.numFrames) {
        if (sequence.loop) {
          anim.currentFrameIndex = 0;
        } else {
          anim.currentFrameIndex = sequence.numFrames - 1;
          anim.isPlaying = false;
        }
      }
    }

    // Only change texture if this animation uses a different sprite sheet
    if (sequence.texture != nullptr && sequence.texture != anim.lastTexture) {
      sprite.texture = sequence.texture;
      anim.lastTexture = sequence.texture;
    }
    
    int framesPerRow = sequence.sheetWidth / sequence.frameWidth;
    int globalFrameIndex = sequence.startIndex + anim.currentFrameIndex;
    sprite.srcRect.x = (globalFrameIndex % framesPerRow) * sequence.frameWidth;
    sprite.srcRect.y = (globalFrameIndex / framesPerRow) * sequence.frameHeight;
    sprite.srcRect.w = sequence.frameWidth;
    sprite.srcRect.h = sequence.frameHeight;
  }
}
