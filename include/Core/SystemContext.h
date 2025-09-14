#pragma once

#include "Core/Packet.h"
#include "Core/ThreadSafeQueue.h"
#include <cstdint>
#include <unordered_map>
#include <string>

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

/**
 * @brief A container for shared game services and managers.
 * @details This struct provides a centralized access point to all major
 * systems and data managers in the game. An instance of this context is passed
 * to each system, allowing them to interact with other parts of the engine
 * without requiring a complex web of direct dependencies. This promotes loose
 * coupling and simplifies system design.
 */
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
  ThreadSafeQueue<RecvPacket>* serverRecvQueue = nullptr; // For incoming packets (both client and server)
  ThreadSafeQueue<SendRequest>* serverSendQueue = nullptr; // For server outgoing packets (needs SendRequest)
  ThreadSafeQueue<PacketPtr>* clientRecvQueue = nullptr;   // For client outgoing packets (only needs PacketPtr)
  ThreadSafeQueue<PacketPtr>* clientSendQueue = nullptr;   // For client outgoing packets (only needs PacketPtr)
  ThreadSafeQueue<MoveApplied>* pendingMoves = nullptr;
  std::unordered_map<clientid_t, std::string>* clientNameMap = nullptr;
  Server* server = nullptr;
  Socket* socket = nullptr;
  bool bIsServer; // 0 for server
};