#ifndef SYSTEM_RENDERSYSTEM_
#define SYSTEM_RENDERSYSTEM_

#include "Core/Type.h"
#include "SDL_ttf.h"

class Registry;
class SDL_Renderer;

class RenderSystem {
  Registry* registry;
  SDL_Renderer* renderer;
  TTF_Font* font;

 public:
  RenderSystem(Registry* r, SDL_Renderer* render, TTF_Font* f);
  void Update();

 private:
  void RenderChunks();
  void RenderEntities();
  void RenderTexts();
  Vec2f GetCameraPosition() const;
  Vec2f WorldToScreen(Vec2f worldPos, Vec2f cameraPos, int screenWidth,
                      int screenHeight) const;
};

#endif /* SYSTEM_RENDERSYSTEM_ */
