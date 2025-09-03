#ifndef CORE_GAMESTATE_
#define CORE_GAMESTATE_

class GEngine;

/**
 * @brief An abstract base class for different states of the game (e.g., menu,
 * playing).
 * @details Defines a simple interface with Enter() and Exit() methods, which
 * are called when the game transitions into or out of a state. This allows for
 * state-specific logic to be cleanly separated.
 */
class IGameState {
 public:
  virtual ~IGameState() = default;
  virtual void Init(GEngine* engine) = 0;
  virtual void Cleanup() = 0;
  virtual void Update(float deltaTime) = 0;

 private:
  virtual void HandleInput() = 0;
  virtual void Render() = 0;
};

#endif /* CORE_GAMESTATE_ */
