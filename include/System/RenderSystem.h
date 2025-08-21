#ifndef SYSTEM_RENDERSYSTEM_
#define SYSTEM_RENDERSYSTEM_

#include "Core/Type.h"
#include "SDL_ttf.h"

class Registry;
class World;
class SDL_Renderer;

class RenderSystem {
  Registry* registry;
  SDL_Renderer* renderer;
  World* world;
  TTF_Font* font;

 public:
  RenderSystem(Registry* r, SDL_Renderer* render, World* world, TTF_Font* f);
  void Update();

 private:
  void RenderChunks(Vec2f cameraPos, Vec2 screenSize);
  void RenderEntities(Vec2f cameraPos, Vec2 screenSize);
  void RenderBuildingPreviews(Vec2f cameraPos, Vec2 screenSize);
  void RenderTexts(Vec2f cameraPos, Vec2 screenSize);
};

#endif /* SYSTEM_RENDERSYSTEM_ */
