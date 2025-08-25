#ifndef CORE_GAMESTATE_
#define CORE_GAMESTATE_

/**
 * @brief An abstract base class for different states of the game (e.g., menu, playing).
 * @details Defines a simple interface with Enter() and Exit() methods, which are
 *          called when the game transitions into or out of a state. This allows
 *          for state-specific logic to be cleanly separated.
 */
class GameState {
 public:
  virtual ~GameState() = default;
  virtual void Enter() = 0;
  virtual void Exit() = 0;
};

/**
 * @brief Represents the main menu state of the game.
 */
class MainMenuState : public GameState {
 public:
  virtual void Enter() override;
  virtual void Exit() override;
};

/**
 * @brief Represents the primary gameplay state.
 */
class PlayState : public GameState {
 public:
  virtual void Enter() override;
  virtual void Exit() override;
};

#endif /* CORE_GAMESTATE_ */
