#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>

#include "CommandQueue.h"
#include "Components/TransformComponent.h"
#include "Components/RefineryComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "GEngine.h"
#include "Entity.h"
#include "Event.h"
#include "GameState.h"
#include "Item.h"
#include "Registry.h"
#include "SDL.h"
#include "SDL_image.h"
#include "System/InputSystem.h"
#include "System/TimerSystem.h"

void GameLoop(GEngine *engine, CommandQueue *queue, std::atomic<bool> &running) {
  // const auto tick = std::chrono::milliseconds(100);
  InputSystem inputSystem(engine, queue, running);
  inputSystem.RegisterInputBindings();
  float currentTime;
  float deltaTime;
  std::chrono::steady_clock::time_point startTimeChrono;
  std::chrono::steady_clock::time_point currentTimeChrono;
  std::chrono::steady_clock::time_point prevTimeChrono;
  startTimeChrono = std::chrono::steady_clock::now();
  currentTimeChrono = prevTimeChrono = startTimeChrono;
  while (running) {
    currentTimeChrono = std::chrono::steady_clock::now();
    deltaTime =
        std::chrono::duration<float, std::chrono::milliseconds::period>(
            currentTimeChrono - prevTimeChrono)
            .count();
    deltaTime /= 1000.f;
    prevTimeChrono = currentTimeChrono;
    currentTime =
        std::chrono::duration<float, std::chrono::milliseconds::period>(
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

int main(int argc, char *argv[]) {
  if ((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1)) {
    std::cout << "Could not initialize SDL:" << SDL_GetError() << ".\n";
    exit(-1);
  }

  std::cout << "SDL initialized.\n";

  SDL_Window *window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, 640, 480, 0);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  std::unique_ptr<GEngine> engine = std::make_unique<GEngine>(window, renderer);
  
  engine->ChangeState(std::make_unique<MainMenuState>());

  // INPUT TESTING
  // engine->GetDispatcher()->Subscribe<StartInteractEvent>(
  //     [](StartInteractEvent e) { std::cout << "start input\n"; });
  // engine->GetDispatcher()->Subscribe<StopInteractEvent>(
  //     [](StopInteractEvent e) { std::cout << "stop input\n"; });

  std::atomic<bool> running = true;
  std::unique_ptr<CommandQueue> commandQueue = std::make_unique<CommandQueue>();
  GameLoop(engine.get(), commandQueue.get(), running);

  // inputThread.join();
  SDL_DestroyWindow(window);
  SDL_Quit();
  exit(0);
}
