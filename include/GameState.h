#pragma once

class GameState {
public:
    virtual ~GameState() = default;
    virtual void Enter() = 0;
    virtual void Exit() = 0;
    virtual void Update() = 0;
};

class MainMenuState : public GameState{
public:
    virtual void Enter() override;
    virtual void Exit() override;
    virtual void Update() override;
};
class PlayState : public GameState{
public:
    virtual void Enter() override;
    virtual void Exit() override;
    virtual void Update() override;
};
