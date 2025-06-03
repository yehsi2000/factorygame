#pragma once
#include <memory>
#include "Component.h"
#include "Player.h"

class ResourceNodeComponent : public Component {
  long long leftResource;
  std::weak_ptr<Player> Miner;

 public:
  ResourceNodeComponent(int totalResource);
  void AddMiner(std::weak_ptr<Player> player);
  void RemoveMiner();
  void Update() override;
  long long leftcount();
};
