#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <unordered_map>

#include "CommandQueue.h"
#include "Entity.h"
#include "Event.h"
#include "InputSystem.h"
#include "Item.h"
#include "World.h"

// void InputThread(std::atomic<bool>& running, World* world,
//                  CommandQueue* queue) {
//   InputSystem inputSystem(world, queue, running);
//   inputSystem.RegisterInputBindings();
//   while (running) {
//     inputSystem.Update();
//   }
// }

void GameLoop(World* world, CommandQueue* queue, std::atomic<bool>& running) {
  //const auto tick = std::chrono::milliseconds(100);
  InputSystem inputSystem(world, queue, running);
  inputSystem.RegisterInputBindings();
  
  while (running) {
    //auto start = std::chrono::steady_clock::now();

    inputSystem.Update();

    // 커맨드 처리
    auto commands = queue->PopAll();
    while (!commands.empty()) {
      auto cmd = std::move(commands.front());
      commands.pop();
      if (cmd) cmd();
    }

    world->Update();

    //std::this_thread::sleep_until(start + tick);
  }
}

int main(int argc, char* argv[]) {
  if ((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1)) {
    std::cout<<"Could not initialize SDL:"<< SDL_GetError() << ".\n";
    exit(-1);
  }

  std::cout<<"SDL initialized.\n";

  SDL_Window* window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);

  std::unique_ptr<World> world = std::make_unique<World>();
  world->ChangeState(std::make_unique<MainMenuState>());

  EntityID resourceNode = world->registry->createEntity();
  EntityID refinery = world->registry->createEntity();
  EntityID player = world->registry->createEntity();
  world->registry->addComponent<ResourceNodeComponent>(resourceNode, 1000,
                                                       OreType::Iron);
  world->registry->addComponent<PositionComponent>(resourceNode, 100., 200.);
  world->registry->addComponent<RefineryComponent>(refinery);
  world->registry->addComponent<PositionComponent>(refinery, 300., 100.);
  world->GetDispatcher()->Subscribe<StartInteractEvent>(
      [](StartInteractEvent e) { std::cout << "start input\n"; });
  world->GetDispatcher()->Subscribe<StopInteractEvent>(
      [](StopInteractEvent e) { std::cout << "stop input\n"; });
  std::atomic<bool> running = true;

  std::unique_ptr<CommandQueue> commandQueue = std::make_unique<CommandQueue>();

  //std::thread inputThread(InputThread, std::ref(running), world.get(), commandQueue.get());
  GameLoop(world.get(), commandQueue.get(), running);

  //inputThread.join();
  SDL_DestroyWindow(window);
  SDL_Quit();
  exit(0);
}
