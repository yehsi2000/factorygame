#pragma once

#include "GameState.h"

#include <vector>
#include <memory>
#include <utility>


class World{
    class EventDispatcher* dispatcher;
    std::vector<std::shared_ptr<class Entity>> entities;
    std::unique_ptr<GameState> currentState;

    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World &&) = delete;
    World& operator=(World&&)= delete;

public: 
    World();
    static World& Instance(){
        static World instance;
        return instance;
    }

    void ChangeState(std::unique_ptr<GameState> newState);
    void Update();

    inline EventDispatcher* GetDispatcher() {return dispatcher;}

    template<typename T, typename... Args>
    std::shared_ptr<T> CreateEntity(Args&&... args){
        auto ent = std::make_shared<T>(std::forward<Args>(args)...);
        entities.push_back(ent);
        return ent;
    }
};