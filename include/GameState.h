﻿#ifndef GAMESTATE_
#define GAMESTATE_

class GameState {
 public:
  virtual ~GameState() = default;
  virtual void Enter() = 0;
  virtual void Exit() = 0;
};

class MainMenuState : public GameState {
 public:
  virtual void Enter() override;
  virtual void Exit() override;
};
class PlayState : public GameState {
 public:
  virtual void Enter() override;
  virtual void Exit() override;
};

#endif /* GAMESTATE_ */
