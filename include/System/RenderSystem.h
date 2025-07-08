#ifndef SYSTEM_RENDERSYSTEM_
#define SYSTEM_RENDERSYSTEM_

#include "Registry.h"
#include "SDL.h"

class RenderSystem {
  Registry* registry;
  SDL_Renderer* renderer;

 public:
  RenderSystem(Registry* r, SDL_Renderer* render);
  void Update();
};

#endif /* SYSTEM_RENDERSYSTEM_ */
