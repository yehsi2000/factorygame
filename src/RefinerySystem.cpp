#include "RefinerySystem.h"

#include "Item.h"
#include "RefineryComponent.h"
#include "Registry.h"

RefinerySystem::RefinerySystem(Registry* r) {
  registry = r;
  // Refinery의 근처에서 Interact했을 때
  // 특정 ore가 3개 이상이면 1개의 ingot으로 변환
}

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
