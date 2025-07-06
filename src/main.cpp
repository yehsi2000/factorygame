#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <unordered_map>

#include "CommandQueue.h"
#include "Engine.h"
#include "Entity.h"
#include "Event.h"
#include "GameState.h"
#include "InputSystem.h"
#include "Item.h"
#include "PositionComponent.h"
#include "RefineryComponent.h"
#include "Registry.h"
#include "ResourceNodeComponent.h"

void GameLoop(Engine* engine, CommandQueue* queue, std::atomic<bool>& running) {
  // const auto tick = std::chrono::milliseconds(100);
  InputSystem inputSystem(engine, queue, running);
  inputSystem.RegisterInputBindings();
  double currentTime;
  double deltaTime;
  std::chrono::steady_clock::time_point startTimeChrono;
  std::chrono::steady_clock::time_point currentTimeChrono;
  std::chrono::steady_clock::time_point prevTimeChrono;
  startTimeChrono = std::chrono::high_resolution_clock::now();
  currentTimeChrono = prevTimeChrono = startTimeChrono;
  while (running) {
    currentTimeChrono = std::chrono::high_resolution_clock::now();
    deltaTime =
        std::chrono::duration<double, std::chrono::milliseconds::period>(
            currentTimeChrono - prevTimeChrono)
            .count();
    deltaTime /= 1000.f;
    prevTimeChrono = currentTimeChrono;
    currentTime =
        std::chrono::duration<double, std::chrono::milliseconds::period>(
            currentTimeChrono - startTimeChrono)
            .count();
    currentTime /= 1000.f;

    inputSystem.Update();
    // 커맨드 처리
    auto commands = queue->PopAll();
    while (!commands.empty()) {
      auto cmd = std::move(commands.front());
      commands.pop();
      if (cmd) cmd();
    }

    engine->Update(deltaTime);
  }
}

int main(int argc, char* argv[]) {
  if ((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1)) {
    std::cout << "Could not initialize SDL:" << SDL_GetError() << ".\n";
    exit(-1);
  }

  std::cout << "SDL initialized.\n";

  SDL_Window* window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, 640, 480, 0);

  std::unique_ptr<Engine> engine = std::make_unique<Engine>();
  engine->ChangeState(std::make_unique<MainMenuState>());

  EntityID resourceNode = engine->GetRegistry()->createEntity();
  EntityID refinery = engine->GetRegistry()->createEntity();
  EntityID player = engine->GetRegistry()->createEntity();
  engine->GetRegistry()->addComponent<ResourceNodeComponent>(resourceNode, 1000,
                                                             OreType::Iron);
  engine->GetRegistry()->addComponent<PositionComponent>(resourceNode, 100.,
                                                         200.);
  engine->GetRegistry()->addComponent<RefineryComponent>(refinery);
  engine->GetRegistry()->addComponent<PositionComponent>(refinery, 300., 100.);
  engine->GetDispatcher()->Subscribe<StartInteractEvent>(
      [](StartInteractEvent e) { std::cout << "start input\n"; });
  engine->GetDispatcher()->Subscribe<StopInteractEvent>(
      [](StopInteractEvent e) { std::cout << "stop input\n"; });
  std::atomic<bool> running = true;

  std::unique_ptr<CommandQueue> commandQueue = std::make_unique<CommandQueue>();

  // std::thread inputThread(InputThread, std::ref(running), engine.get(),
  // commandQueue.get());
  GameLoop(engine.get(), commandQueue.get(), running);

  // inputThread.join();
  SDL_DestroyWindow(window);
  SDL_Quit();
  exit(0);
}
