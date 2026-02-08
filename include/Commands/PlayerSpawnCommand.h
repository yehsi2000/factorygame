#pragma once
 

#include "Commands/Command.h"
#include "Core/EventDispatcher.h"
#include "Core/Packet.h"
#include "Core/Registry.h"
#include "Core/World.h"

class PlayerSpawnCommand : public Command {
 public:
  PlayerSpawnCommand(clientid_t clientID, bool isLocalPlayer)
      : clientID(clientID), isLocalPlayer(isLocalPlayer) {};

  ~PlayerSpawnCommand() = default;

  void Execute(Registry* registry, EventDispatcher* eventDispatcher,
               World* world) override {
    world->GeneratePlayer(clientID, {}, isLocalPlayer);
  }

 private:
  clientid_t clientID;
  bool isLocalPlayer;
};
