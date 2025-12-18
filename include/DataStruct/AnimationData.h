#pragma once

#include "SDL_render.h"

enum class AnimationName : int {
  PLAYER_IDLE = 0,
  PLAYER_WALK,
  PLAYER_MINE_RIGHT,
  PLAYER_MINE_DOWN,

  ASSEMBLING_MACHINE_IDLE,
  ASSEMBLING_MACHINE_WORKING,

  DRILL_IDLE,
  DRILL_WORKING,

  MAX_ANIMATIONS
};

// single anim sequence info
struct AnimationSequence {
  int startIndex;  // start frame index of this animation in the sprite sheet
  int numFrames;   // Total # of frames that make up this animation
  float frameRate;
  int frameWidth;  // single frame size in spritesheet
  int frameHeight;
  bool bIsLoop;
  int sheetWidth;
  int sheetHeight;
  SDL_Texture *texture;  // Cached texture reference
};