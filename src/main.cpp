#include <easy/profiler.h>

#include <chrono>
#include <iostream>
#include <memory>

#include "Core/GEngine.h"
#include "Core/World.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

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
    deltaTime = std::chrono::duration<float, std::chrono::milliseconds::period>(
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

void InitMap(GEngine *engine) {
  // registry->AddComponent<TransformComponent>();
}

int main(int argc, char *argv[]) {
  if ((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1) || (TTF_Init() == -1)) {
    std::cout << "Could not initialize SDL:" << SDL_GetError() << ".\n";
    exit(-1);
  }

  std::cout << "SDL initialized " << SDL_GetTicks() << std::endl;

  SDL_Window *window =
      SDL_CreateWindow("FactoryGame", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 1280, 960, SDL_WINDOW_RESIZABLE);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  std::cout << "SDL window created " << SDL_GetTicks() << std::endl;
  TTF_Font *font = TTF_OpenFont("C:\\Windows\\Fonts\\gulim.ttc", 16);
  if (font == NULL) {
    printf("Could not open font! (%s)\n", TTF_GetError());
    return -1;
  }

  std::cout << "font file opened " << SDL_GetTicks() << std::endl;

  std::unique_ptr<GEngine> engine =
      std::make_unique<GEngine>(window, renderer, font);

  engine->ChangeState(std::make_unique<MainMenuState>());

  InitMap(engine.get());

  GameLoop(engine.get());

  // inputThread.join();
  TTF_CloseFont(font);
  SDL_DestroyWindow(window);
  SDL_Quit();
  exit(0);
}