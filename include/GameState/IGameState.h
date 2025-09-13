#ifndef GAMESTATE_IGAMESTATE_
#define GAMESTATE_IGAMESTATE_

class GEngine;

/**
 * @brief An abstract base class for different states of the game (e.g., menu,
 * playing).
 * @details Defines a simple interface with Init() and Cleanup() methods, which
 * are called when the game transitions into or out of a state. This allows for
 * state-specific logic to be cleanly separated.
 */
 
class IGameState {
 public:
  virtual ~IGameState() = default;
  virtual void Init(GEngine* engine) = 0;
  virtual void Cleanup() = 0;
  virtual void Update(float deltaTime) = 0;
};

#endif/* GAMESTATE_IGAMESTATE_ */
