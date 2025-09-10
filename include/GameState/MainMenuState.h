#ifndef GAMESTATE_MAINMENUSTATE_
#define GAMESTATE_MAINMENUSTATE_

#include "GameState/IGameState.h"

class GEngine;
struct SDL_Window;
struct SDL_Renderer;
struct TTF_Font;


/**
 * @brief Represents the main menu state of the game.
 */
class MainMenuState : public IGameState {
 public:
  virtual void Init(GEngine* engine) override;
  virtual void Cleanup() override;
  virtual void Update(float deltaTime) override;

 private:
  GEngine* gEngine;
  SDL_Window* gWindow;
  SDL_Renderer* gRenderer;
  TTF_Font* gFont;
};

#endif /* GAMESTATE_MAINMENUSTATE_ */
