#include <iostream>

#include "Core/GEngine.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "imgui.h"
#include "imgui_freetype.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#define DRAW_DEBUG_RECTS

int main(int argc, char *argv[]) {
  if ((SDL_Init(SDL_INIT_VIDEO) == -1) || (TTF_Init() == -1)) {
    std::cout << "Could not initialize SDL:" << SDL_GetError() << ".\n";
    exit(-1);
  }

  SDL_Window *window = SDL_CreateWindow("FactoryGame", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
  if (window == nullptr) {
    std::cout << "Could not create window:" << SDL_GetError() << ".\n";
    exit(-1);
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) {
    std::cout << "Could not create renderer:" << SDL_GetError() << ".\n";
    exit(-1);
  }

  SDL_StopTextInput();

  // Imgui init
  IMGUI_CHECKVERSION();
  ImGuiContext *uiCtx = ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.Fonts->FontLoaderFlags = ImGuiFreeTypeBuilderFlags_Bold;

  TTF_Font *font = TTF_OpenFont(R"(assets\fonts\NotoSansKR-VF.ttf)", 16);
  if (font == nullptr) {
    TTF_Font *font = TTF_OpenFont(R"(C:\Windows\Fonts\Gulim.ttc)", 16);
    if (font == nullptr) {
      printf("Could not open font! (%s)\n", TTF_GetError());
      return -1;
    } else {
      io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\Gulim.ttc)");
    }
  } else {
    io.Fonts->AddFontFromFileTTF(R"(assets\fonts\NotoSansKR-VF.ttf)");
  }

  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer2_Init(renderer);

  // Start engine
  try {
    GEngine engine(window, renderer, font);
    engine.Run();
  } catch (...) {
    std::cerr << "Engine initialization failed!" << std::endl;
  }

  // Close all system
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