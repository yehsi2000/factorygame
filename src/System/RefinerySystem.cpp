#include "System/RefinerySystem.h"

#include "Components/RefineryComponent.h"
#include "Core/Item.h"
#include "Core/Registry.h"

RefinerySystem::RefinerySystem(Registry* r) { registry = r; }

void RefinerySystem::Update() {
  if (registry == nullptr) return;
}

void RefinerySystem::ConnectMiner(RefineryComponent& refinery,
                                  EntityID player) {
  refinery.connectedMiner = player;
}

void RefinerySystem::DisconnectMiner(RefineryComponent& refinery) {
  refinery.connectedMiner = 0;
}
