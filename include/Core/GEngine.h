#ifndef CORE_GENGINE_
#define CORE_GENGINE_

#include <memory>
#include <vector>

#include "Core/Entity.h"
#include "GameState/IGameState.h"
#include "SDL_ttf.h"
#include "imgui.h"

class AssetManager;
class WorldAssetManager;
class InputManager;

class GEngine {
  SDL_Window *gWindow;
  SDL_Renderer *gRenderer;
  TTF_Font *gFont;

  std::unique_ptr<AssetManager> assetManager;
  std::unique_ptr<WorldAssetManager> worldAssetManager;
  std::unique_ptr<InputManager> inputManager;

  bool bIsRunning = true;

  GEngine(const GEngine &) = delete;
  GEngine &operator=(const GEngine &) = delete;
  GEngine(GEngine &&) = delete;
  GEngine &operator=(GEngine &&) = delete;
  std::vector<std::unique_ptr<IGameState>> gameStates;

 public:
  GEngine(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font);
  ~GEngine();

  void PushState(std::unique_ptr<IGameState> state);
  void PopState();
  void ChangeState(std::unique_ptr<IGameState> state);

  void Run();

  inline SDL_Renderer *GetRenderer() { return gRenderer; }
  inline SDL_Window *GetWindow() { return gWindow; }
  inline TTF_Font *GetFont() { return gFont; }
  inline AssetManager *GetAssetManager() { return assetManager.get(); }
  inline WorldAssetManager *GetWorldAssetManager() {
    return worldAssetManager.get();
  }
  inline InputManager *GetInputManager() { return inputManager.get(); }
  inline void Stop() { bIsRunning = false; }
};

#endif/* CORE_GENGINE_ */
