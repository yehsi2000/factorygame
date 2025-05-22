#include "Player.h"
#include "World.h"

Player::Player() {
  EntityID id = getID();
  inv = std::make_unique<Inventory>();
  OreEventId = World::Instance().GetDispatcher()->Subscribe<OreMinedEvent>([=](const OreMinedEvent& evt){
    std::cout<<"Player "<< id << " mined " << evt.amount << " ores\n";
    inv->add(Item::Ore,evt.amount);
  });
}

Player::~Player(){
  World::Instance().GetDispatcher()->Unsubscribe<OreMinedEvent>(OreEventId);
}

Inventory* Player::GetInventory() const {
  return inv.get();
}
