#ifndef GAMESTATE_MAINMENUSTATE_
#define GAMESTATE_MAINMENUSTATE_

#include "Core/IGameState.h"

class GEngine;

/**
 * @brief Represents the main menu state of the game.
 */
class MainMenuState : public IGameState {
 public:
  virtual void Init(GEngine* engine) override;
  virtual void Cleanup() override;
  virtual void Update(float deltaTime) override;

 private:
  virtual void HandleInput() override;
  virtual void Render() override;
};

#endif /* GAMESTATE_MAINMENUSTATE_ */
