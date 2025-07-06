#include "Entity.h"

class Registry;
struct RefineryComponent;

class RefinerySystem {
  Registry* registry;

 public:
  RefinerySystem(Registry* r);
  void Update();
  void ConnectMiner(RefineryComponent& refinery, EntityID player);
  void DisconnectMiner(RefineryComponent& refinery);
};