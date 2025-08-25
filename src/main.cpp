#include <chrono>
#include <iostream>
#include <memory>

#include "Core/GEngine.h"
#include "Core/GameState.h"
#include "Core/World.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "imgui.h"
#include "imgui_freetype.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#define DRAW_DEBUG_RECTS

void GameLoop(GEngine *engine) {
  using namespace std::chrono;

  steady_clock::time_point startTime;
  steady_clock::time_point curTime;
  steady_clock::time_point prevTime;
  float deltaTime;

  startTime = steady_clock::now();
  curTime = prevTime = startTime;

  while (engine->IsRunning()) {
    curTime = steady_clock::now();
    deltaTime =
        duration<float, milliseconds::period>(curTime - prevTime).count();
    deltaTime /= 1000.f;
    prevTime = curTime;

    engine->Update(deltaTime);
  }
}

int main(int argc, char *argv[]) {
  if ((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1) || (TTF_Init() == -1)) {
    std::cout << "Could not initialize SDL:" << SDL_GetError() << ".\n";
    exit(-1);
  }

  SDL_Window *window = SDL_CreateWindow("FactoryGame", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, 1280, 720, 0);

  if (window == NULL) {
    std::cout << "Could not create window:" << SDL_GetError() << ".\n";
    exit(-1);
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

  if (renderer == NULL) {
    std::cout << "Could not create renderer:" << SDL_GetError() << ".\n";
    exit(-1);
  }

  TTF_Font *font = TTF_OpenFont("C:\\Windows\\Fonts\\NotoSansKR-VF.ttf", 16);
  if (font == NULL) {
    printf("Could not open font! (%s)\n", TTF_GetError());
    return -1;
  }

  SDL_StopTextInput();

  IMGUI_CHECKVERSION();
  ImGuiContext *uiCtx = ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.Fonts->FontLoaderFlags = ImGuiFreeTypeBuilderFlags_Bold;
  io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\NotoSansKR-VF.ttf");

  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer2_Init(renderer);

  std::unique_ptr<GEngine> engine =
      std::make_unique<GEngine>(window, renderer, font);

  engine->ChangeState(std::make_unique<PlayState>());

  GameLoop(engine.get());

  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  TTF_CloseFont(font);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_Quit();
  SDL_Quit();
  return 0;
}