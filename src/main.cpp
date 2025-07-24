#include <chrono>
#include <iostream>
#include <memory>
#include <easy/profiler.h>

#include "GEngine.h"
#include "SDL.h"
#include "SDL_image.h"
#include "World.h"

void GameLoop(GEngine *engine) {
  // const auto tick = std::chrono::milliseconds(100);
  float currentTime;
  float deltaTime;
  std::chrono::steady_clock::time_point startTimeChrono;
  std::chrono::steady_clock::time_point currentTimeChrono;
  std::chrono::steady_clock::time_point prevTimeChrono;

  startTimeChrono = std::chrono::steady_clock::now();
  currentTimeChrono = prevTimeChrono = startTimeChrono;
  while (engine->IsRunning()) {
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
    engine->Update(deltaTime);
  }
}

void InitMap(GEngine* engine){

  //registry->AddComponent<TransformComponent>();
}

int main(int argc, char *argv[]) {

  if ((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1)) {
    std::cout << "Could not initialize SDL:" << SDL_GetError() << ".\n";
    exit(-1);
  }

  std::cout << "SDL initialized.\n";

  SDL_Window *window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_RESIZABLE);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  std::unique_ptr<GEngine> engine = std::make_unique<GEngine>(window, renderer);
  
  engine->ChangeState(std::make_unique<MainMenuState>());



  InitMap(engine.get());

  GameLoop(engine.get());

  // inputThread.join();
  SDL_DestroyWindow(window);
  SDL_Quit();
  exit(0);
}