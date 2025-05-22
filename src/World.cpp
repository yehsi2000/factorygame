#include "World.h"
#include "Event.h"
#include "GameState.h"

World::World() {
    dispatcher = new EventDispatcher();
}

void World::ChangeState(std::unique_ptr<GameState> newState) {
    if(currentState) currentState->Exit();
    currentState = std::move(newState);
    if(currentState) currentState->Enter();
}

void World::Update(){
    if(currentState) currentState->Update();
}
