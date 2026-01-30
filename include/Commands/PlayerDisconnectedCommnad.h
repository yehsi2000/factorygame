#pragma once
 

#include "Commands/Command.h"
#include "Components/InventoryComponent.h"
#include "Components/TransformComponent.h"
#include "Core/EventDispatcher.h"
#include "Core/Entity.h"
#include "Core/Event.h"
#include "Core/Item.h"
#include "Core/Packet.h"
#include "Core/Registry.h"
#include "Core/World.h"

class PlayerDisconnectedCommand : public Command {
 public:
  explicit PlayerDisconnectedCommand(clientid_t clientID)
      : clientID(clientID) {};

  ~PlayerDisconnectedCommand() = default;

  void Execute(Registry* registry, EventDispatcher* eventDispatcher,
               World* world) override {
    Entity disconnected = world->GetPlayerByClientID(clientID);
    if (disconnected == Entity::Null()) return;
    if (registry->HasComponent<InventoryComponent>(disconnected)) {
      auto& inv = registry->GetComponent<InventoryComponent>(disconnected);
      for (const auto& [itemID, amount] : inv.items) {
        eventDispatcher->Publish(ItemDropInWorldEvent(registry->GetComponent<TransformComponent>(disconnected).position,
            ItemPayload{0, disconnected, itemID, amount}));
      }
    }
    registry->DestroyEntity(disconnected);
  }

 private:
  clientid_t clientID;
};
