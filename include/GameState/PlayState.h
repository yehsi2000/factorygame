#ifndef GAMESTATE_PLAYSTATE_
#define GAMESTATE_PLAYSTATE_

#include "Core/IGameState.h"

class GEngine;

/**
 * @brief Represents the primary gameplay state.
 */
class PlayState : public IGameState {
 public:
  virtual void Init(GEngine* engine) override;
  virtual void Cleanup() override;
  virtual void Update(float deltaTime) override;

 private:
  virtual void HandleInput() override;
  virtual void Render() override;
};

#endif /* GAMESTATE_PLAYSTATE_ */
