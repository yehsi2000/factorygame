#ifndef GAMESTATE_PAUSESTATE_
#define GAMESTATE_PAUSESTATE_

#include "GameState/IGameState.h"

class GEngine;

/**
 * @brief Represents the pause state of the game.
 */

 class PauseState : public IGameState {
  public:
    virtual void Init(GEngine* engine) override;
    virtual void Cleanup() override;
    virtual void Update(float deltaTime) override;
 };

#endif/* GAMESTATE_PAUSESTATE_ */
