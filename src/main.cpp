#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <unordered_map>

#include "CommandQueue.h"
#include "Entity.h"
#include "Item.h"
#include "Event.h"
#include "InputSystem.h"
#include "RefineryComponent.h"
#include "ResourceNodeComponent.h"
#include "World.h"

void InputThread(std::atomic<bool>& running, World* world, CommandQueue* queue) {
  InputSystem inputSystem(world, queue);
  inputSystem.RegisterInputBindings();
  while (running) {
    inputSystem.Update();
  }
}

void GameLoop(World* world, CommandQueue* queue, std::atomic<bool>& running) {
    const auto tick = std::chrono::milliseconds(1000);
  while (running) {
    auto start = std::chrono::steady_clock::now();

    // 커맨드 처리
    auto commands = queue->PopAll();
    while (!commands.empty()) {
      auto cmd = std::move(commands.front());
      commands.pop();
      if (cmd) cmd();
    }

    world->Update();

    std::this_thread::sleep_until(start + tick);
  }
}

int main(int argc, char* argv[]) {
  std::unique_ptr<World> world = std::make_unique<World>();
  world->ChangeState(std::make_unique<MainMenuState>());

  EntityID resourceNode = world->registry->createEntity();
  EntityID refinery = world->registry->createEntity();
  EntityID player = world->registry->createEntity();
  world->registry->addComponent<ResourceNodeComponent>(resourceNode, 1000, OreType::Iron);
  world->registry->addComponent<RefineryComponent>(refinery);

  std::atomic<bool> running = true;

  std::unique_ptr<CommandQueue> commandQueue = std::make_unique<CommandQueue>();

  std::thread inputThread(InputThread, std::ref(running), world.get(), commandQueue.get());
  GameLoop(world.get(), commandQueue.get(), running);

  inputThread.join();

  return 0;
}
