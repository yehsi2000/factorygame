#ifndef SYSTEM_UISYSTEM_
#define SYSTEM_UISYSTEM_

#include "SDL.h"
#include "imgui.h"

class UISystem {
 public:
  UISystem(SDL_Renderer* r, ImGuiIO& io);
  void Update();

 private:
  SDL_Renderer* renderer;
  ImGuiIO& io;
};

#endif /* SYSTEM_UISYSTEM_ */
