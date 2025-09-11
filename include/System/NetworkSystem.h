#ifndef SYSTEM_NETWORKSYSTEM_
#define SYSTEM_NETWORKSYSTEM_

#include <memory>
#include <string>

#include "Core/SystemContext.h"


class EventHandle;

class NetworkSystem {
  /**
   * //TODO : Networking prototype
   * simple things, like sending player position to server,
   * or broadcasting entity, chunk creation to client whatever
   */
  AssetManager* assetManager;
  EventDispatcher* eventDispatcher;
  Registry* registry;
  TimerManager* timerManager;
  World* world;

 public:
  NetworkSystem(const SystemContext& context);
  ~NetworkSystem();
  void Update(float deltatime);

 private:
  std::unique_ptr<EventHandle> sendChatHandle;
  void SendChat(std::shared_ptr<std::string> chat);
};

#endif /* SYSTEM_NETWORKSYSTEM_ */
