#include "Entity.h"
#include "Player.h"
#include "ResourceNodeComponent.h"
#include "RefineryComponent.h"
#include "Event.h"
#include "World.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <unordered_map>

void GetInput(std::atomic<bool>& running, std::atomic<int>& inputval) {
  int a = 0;
  while (running && std::cin >> a) {
    if(a == -1) running = false;
    inputval = a;
  }
}

int main() {
  World::Instance().ChangeState(std::make_unique<MainMenuState>());

  std::unordered_map<EntityID, std::shared_ptr<Entity>> Entities;
  std::shared_ptr<Entity> resourceNode = std::make_shared<Entity>();
  std::shared_ptr<Entity> refinery = std::make_shared<Entity>();
  std::shared_ptr<Player> player = std::make_shared<Player>();

  auto rcn = resourceNode->AddComponent<ResourceNodeComponent>(1000);
  auto rfc = refinery->AddComponent<RefineryComponent>();

  rcn->AddMiner(player);
  rfc->ConnectMiner(player);

  Entities.insert({resourceNode->getID(), resourceNode});
  Entities.insert({refinery->getID(), refinery});
  Entities.insert({player->getID(), player});

  const auto tick = std::chrono::milliseconds(1000);
  std::atomic<bool> running = true;
  std::atomic<int> inputval = 0;
  std::thread inputThread(GetInput, std::ref(running), std::ref(inputval));

  while (running) {
    auto start = std::chrono::system_clock::now();
    for (auto& [id, e] : Entities) e->update();

    if(inputval > 0){
      switch(inputval){
        case 1: 
          rcn->RemoveMiner();
        break;
        case 2: 
          rfc->DisconnectMiner();
        break;
      }
      inputval = 0;
    }

    std::this_thread::sleep_until(start + tick);
  }

  inputThread.join();
  return 0;
}
