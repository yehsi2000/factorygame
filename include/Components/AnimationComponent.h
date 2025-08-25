#ifndef COMPONENTS_ANIMATIONCOMPONENT_
#define COMPONENTS_ANIMATIONCOMPONENT_

#include "SDL_render.h"
#include <map>

enum class AnimationName : int {
  PLAYER_IDLE = 0,
  PLAYER_WALK,
  PLAYER_MINE_RIGHT,
  PLAYER_MINE_DOWN,

  ASSEMBLING_MACHINE_IDLE,
  ASSEMBLING_MACHINE_WORKING,

  DRILL_IDLE,
  DRILL_WORKING,
};

// single anim sequence info
struct AnimationSequence {
  int startIndex; // start frame index of this animation in the sprite sheet
  int numFrames;  // Total # of frames that make up this animation
  float frameRate;
  int frameWidth; // single frame size in spritesheet
  int frameHeight;
  bool loop;
  int sheetWidth;
  int sheetHeight;
  SDL_Texture *texture; // Cached texture reference
};

struct AnimationComponent {
  std::map<AnimationName, AnimationSequence> animations;
  AnimationName currentAnimation;

  int currentFrameIndex = 0;
  float frameTimer = 0.f;
  bool isPlaying = true;
  SDL_Texture *lastTexture = nullptr;
};

#endif /* COMPONENTS_ANIMATIONCOMPONENT_ */
