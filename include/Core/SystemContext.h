#pragma once

#include "Core/Packet.h"
#include "Core/ThreadSafeQueue.h"
#include <cstdint>

// Forward declarations
class AssetManager;
class WorldAssetManager;
class CommandQueue;
class Registry;
class EventDispatcher;
class World;
class Server;
class InputManager;
class EntityFactory;
class TimerManager;
class Socket;

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
  ThreadSafeQueue<PacketPtr>* packetQueue = nullptr; // For incoming packets (both client and server)
  ThreadSafeQueue<SendRequest>* serverSendQueue = nullptr; // For server outgoing packets (needs SendRequest)
  ThreadSafeQueue<PacketPtr>* clientSendQueue = nullptr;   // For client outgoing packets (only needs PacketPtr)
  Server* server = nullptr;
  Socket* socket = nullptr;
  uint64_t clientID = 0;
};