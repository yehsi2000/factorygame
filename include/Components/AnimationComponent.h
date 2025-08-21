#ifndef COMPONENTS_ANIMATIONCOMPONENT_
#define COMPONENTS_ANIMATIONCOMPONENT_

#include <map>
#include <string>

// single anim sequence info (ex: "walk", "idle", "attack")
struct AnimationSequence {
  int startIndex;  // start frame index of this animation in the sprite sheet
  int numFrames;   // Total # of frames that make up this animation
  float frameRate;
  int frameWidth = 16;  // single frame size in spritesheet
  int frameHeight = 16;
  bool loop = true;
};

struct AnimationComponent {
  std::map<std::string, AnimationSequence> animations;
  std::string currentAnimationName;
  
  int currentFrameIndex = 0;
  float frameTimer = 0.f;
  bool isPlaying = true;
};

#endif /* COMPONENTS_ANIMATIONCOMPONENT_ */
