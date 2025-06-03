#include "ResourceNodeComponent.h"
#include "World.h"
#include "Event.h"

#include <iostream>
#include <typeinfo>

ResourceNodeComponent::ResourceNodeComponent(int totalResource) {
  leftResource = totalResource;
}

void ResourceNodeComponent::AddMiner(std::weak_ptr<Player> player) {
  Miner = player;
  std::cout << typeid(*player.lock()).name() << " added to " << typeid(*this).name() << std::endl;
}

void ResourceNodeComponent::RemoveMiner() {
  std::cout << typeid(*Miner.lock()).name() << " removed from " << typeid(*this).name() << std::endl;
  Miner.reset();
}

long long ResourceNodeComponent::leftcount() {
  return leftResource;
}

void ResourceNodeComponent::Update() {
  if (auto miner = Miner.lock()) {
    World::Instance().GetDispatcher()->Dispatch(OreMinedEvent{miner->getID(),3});
  }
}
