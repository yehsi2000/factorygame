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

// struct AnimationComponent {
//   AnimationName currentAnim;
//   int currentFrameIndex;
//   float frameTimer;
//   bool isPlaying;
// };