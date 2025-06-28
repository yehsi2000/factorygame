#include "RefinerySystem.h"
#include "Item.h"
#include "World.h"

RefinerySystem::RefinerySystem() {
  
}

void RefinerySystem::Update(World& world) {
  
}

void RefinerySystem::ConnectMiner(RefineryComponent& refinery,
                                  EntityID player) {
  refinery.connectedMiner = player;
}

void RefinerySystem::DisconnectMiner(RefineryComponent& refinery) {
  refinery.connectedMiner = 0;
}
