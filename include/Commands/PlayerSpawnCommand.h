#ifndef COMMANDS_PLAYERSPAWNCOMMAND_
#define COMMANDS_PLAYERSPAWNCOMMAND_

#include <algorithm>
#include <cstring>

#include "Commands/Command.h"
#include "Components/InventoryComponent.h"
#include "Core/EventDispatcher.h"
#include "Core/Item.h"
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
    world->GeneratePlayer(clientID, bIsLocalPlayer);
  }

 private:
  clientid_t clientID;
  bool bIsLocalPlayer;
};

#endif/* COMMANDS_PLAYERSPAWNCOMMAND_ */
