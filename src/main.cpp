#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <unordered_map>

#include "CommandQueue.h"
#include "Entity.h"
#include "Event.h"
#include "Player.h"
#include "RefineryComponent.h"
#include "ResourceNodeComponent.h"
#include "World.h"

std::unordered_map<EntityID, std::shared_ptr<Entity>> Entities;

void GetInput(std::atomic<bool>& running, std::atomic<int>& inputval) {
  int a = 0;
  while (running && std::cin >> a) {
    if (a == -1) running = false;
    inputval = a;
  }
}

void InputThread(CommandQueue& queue, std::atomic<bool>& running) {
  int input;
  while (running) {
    std::cin >> input;
    queue.Push([input]() {
      if (input == 1) {
        std::cout << "User pressed 1: Stop mining.\n";
      }
    });
  }
}

void GameLoop(CommandQueue& queue, std::atomic<bool>& running) {
  using namespace std::chrono_literals;
  while (running) {
    auto start = std::chrono::steady_clock::now();

    // 커맨드 처리
    while (!queue.Empty()) {
      auto cmd = queue.Pop();
      if (cmd) cmd();  // 실행
    }

    // 컴포넌트 업데이트
    for (auto& [id, entity] : Entities) {
      entity->update();
    }

    std::this_thread::sleep_until(start + 100ms);
  }
}
int main() {
  World::Instance().ChangeState(std::make_unique<MainMenuState>());
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

  CommandQueue commandQueue;
  GameLoop(commandQueue, running);

  inputThread.join();
  

  return 0;
}
