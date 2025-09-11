#ifndef SYSTEM_RENDERSYSTEM_
#define SYSTEM_RENDERSYSTEM_

#include "Core/Type.h"
#include "Core/SystemContext.h"
#include "SDL_ttf.h"

struct SDL_Renderer;

/**
 * @brief Responsible for rendering every entity, chunk and text
 * 
 */
class RenderSystem {
  Registry *registry;
  SDL_Renderer *renderer;
  World *world;
  TTF_Font *font;

public:
  RenderSystem(const SystemContext& context, SDL_Renderer* renderer, TTF_Font *f);
  ~RenderSystem();

  void Update();

private:
  void RenderChunks(Vec2f cameraPos, Vec2 screenSize, float zoom);
  void RenderEntities(Vec2f cameraPos, Vec2 screenSize, float zoom);
  void RenderBuildingPreviews(Vec2f cameraPos, Vec2 screenSize, float zoom);
  void RenderTexts(Vec2f cameraPos, Vec2 screenSize, float zoom);
  void RenderDebugRect(Vec2f cameraPos, Vec2 screenSize, float zoom);

  bool IsOffScreen(Vec2f screenPos, Vec2 screenSize, Vec2f entitySize);
};

#endif/* SYSTEM_RENDERSYSTEM_ */
