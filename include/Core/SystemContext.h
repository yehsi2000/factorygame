#pragma once

// Forward declarations
class AssetManager;
class WorldAssetManager;
class CommandQueue;
class Registry;
class EventDispatcher;
class World;
class InputManager;
class EntityFactory;
class TimerManager;

struct SystemContext {
    AssetManager* assetManager = nullptr;
    WorldAssetManager* worldAssetManager = nullptr;
    CommandQueue* commandQueue = nullptr;
    Registry* registry = nullptr;
    EventDispatcher* eventDispatcher = nullptr;
    World* world = nullptr;
    InputManager* inputManager = nullptr;
    EntityFactory* entityFactory = nullptr;
    TimerManager* timerManager = nullptr;
};