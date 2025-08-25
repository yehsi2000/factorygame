#include "Util/AnimUtil.h"

#include "Components/AnimationComponent.h"

#include "SDL.h"
#include <utility>

namespace util {

void SetAnimation(AnimationName name, AnimationComponent &animComp,
                  bool bPlay) {
  if (animComp.currentAnimation != name) {
    animComp.currentAnimation = name;
    animComp.currentFrameIndex = 0;
    animComp.frameTimer = 0.f;
  }
  animComp.isPlaying = bPlay;
}

void AddAnimation(AnimationComponent &animComp, const AnimationName &animName,
                  SDL_Texture *texture, AnimationSequence &&animSequence) {

  animComp.animations[animName] = std::move(animSequence);
  animComp.animations[animName].texture = texture;
  int sheetWidth, sheetHeight;
  SDL_QueryTexture(texture, NULL, NULL, &sheetWidth, &sheetHeight);
  animComp.animations[animName].sheetWidth = sheetWidth;
  animComp.animations[animName].sheetHeight = sheetHeight;
}

} // namespace util