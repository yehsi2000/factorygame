#pragma once
#include <memory>
#include <iostream>
#include <typeinfo>
#include "Component.h"
#include "Player.h"

class RefineryComponent : public Component {
  std::weak_ptr<Player> connectedMiner;

 public:
  void ConnectMiner(std::weak_ptr<Player> player);
  void DisconnectMiner();
  void Update() override;
};
