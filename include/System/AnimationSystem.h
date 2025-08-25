#ifndef SYSTEM_ANIMATIONSYSTEM_
#define SYSTEM_ANIMATIONSYSTEM_

class Registry;
struct SDL_Renderer;

class AnimationSystem {
  Registry* registry;
  SDL_Renderer* renderer;

 public:
  AnimationSystem(Registry* r, SDL_Renderer* renderer);
  void Update(float deltaTime);
};

#endif /* SYSTEM_ANIMATIONSYSTEM_ */
