#pragma once
#include "RefineryComponent.h"

class RefinerySystem {

 public:
  RefinerySystem();
  void Update(class World& world);
  void ConnectMiner(RefineryComponent& refinery, EntityID player);
  void DisconnectMiner(RefineryComponent& refinery);
};