#include "Util/AnimUtil.h"

#include "Components/AnimationComponent.h"

namespace util {

void SetAnimation(AnimationName name, AnimationComponent &animComp,
                  bool bPlay) {
  if (animComp.currentAnimation != name) {
    animComp.currentAnimation = name;
    animComp.currentFrameIndex = 0;
    animComp.frameTimer = 0.f;
  }
  animComp.bIsPlaying = bPlay;
}

// TODO : should store animation data somewhere else like asset

void AddAnimation(AnimationComponent &animComp, const AnimationName &animName,
                  SDL_Texture *texture, AnimationSequence &&animSequence) {
  auto animIdx = static_cast<std::size_t>(animName);
  animComp.animations[animIdx] = std::move(animSequence);
  animComp.animations[animIdx].texture = texture;
  int sheetWidth, sheetHeight;
  SDL_QueryTexture(texture, NULL, NULL, &sheetWidth, &sheetHeight);
  animComp.animations[animIdx].sheetWidth = sheetWidth;
  animComp.animations[animIdx].sheetHeight = sheetHeight;
}

}  // namespace util