#pragma once
 

#include "Commands/Command.h"
#include "Core/EventDispatcher.h"
#include "Core/Packet.h"
#include "Core/Registry.h"
#include "Core/World.h"

class PlayerSpawnCommand : public Command {
 public:
  PlayerSpawnCommand(clientid_t clientID, bool bIsLocalPlayer)
      : clientID(clientID), bIsLocalPlayer(bIsLocalPlayer) {};

  ~PlayerSpawnCommand() = default;

  void Execute(Registry* registry, EventDispatcher* eventDispatcher,
               World* world) override {
    world->GeneratePlayer(clientID, {}, bIsLocalPlayer);
  }

 private:
  clientid_t clientID;
  bool bIsLocalPlayer;
};
