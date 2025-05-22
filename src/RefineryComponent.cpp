#include "RefineryComponent.h"
#include "Inventory.h"
#include "World.h"

void RefineryComponent::ConnectMiner(std::weak_ptr<Player> player) {
  connectedMiner = player;
  std::cout << typeid(*player.lock()).name() << " connected to " << typeid(this).name() << std::endl;
}

void RefineryComponent::DisconnectMiner() {
  std::cout << typeid(*connectedMiner.lock()).name() << " disconnected from " << typeid(this).name() << std::endl;
  connectedMiner.reset();
}

void RefineryComponent::Update() {
  if (auto miner = connectedMiner.lock()) {
    if (Inventory* inv = miner->GetInventory()) {
      if (inv->get(Item::Ore) >= 3) {
        inv->consume(Item::Ore, 3);
        inv->add(Item::Ingot, 1);
        std::cout << typeid(*miner).name() << " forged 1 ingot ore left:" << inv->get(Item::Ore) << std::endl;
      }
    }
  }
}
