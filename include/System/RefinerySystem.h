#ifndef SYSTEM_REFINERYSYSTEM_
#define SYSTEM_REFINERYSYSTEM_

#include "Components/RefineryComponent.h"
#include "Core/Entity.h"

class Registry;

class RefinerySystem {
  Registry* registry;

 public:
  RefinerySystem(Registry* r);
  void Update();
  void ConnectMiner(RefineryComponent& refinery, EntityID player);
  void DisconnectMiner(RefineryComponent& refinery);
};

#endif /* SYSTEM_REFINERYSYSTEM_ */
