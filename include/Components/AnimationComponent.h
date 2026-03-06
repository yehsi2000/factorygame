#pragma once

#include <array>

#include "DataStruct/AnimationData.h"

struct AnimationComponent {
  std::array<AnimationSequence,
             static_cast<size_t>(AnimationName::MAX_ANIMATIONS)>
      animations;
  AnimationName currentAnimation;

  int currentFrameIndex;
  float frameTimer;
  bool isPlaying;
  SDL_Texture *lastTexture;
};

// TODO : Let animation manager hold the data. Component just points it.

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<AnimationComponent> == true);
static_assert(std::is_trivially_destructible_v<AnimationComponent> == true);
#endif