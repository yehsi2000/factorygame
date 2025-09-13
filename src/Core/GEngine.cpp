#include "Core/GEngine.h"

#include <cassert>
#include <chrono>
#include <tuple>
#include <utility>

#include "Core/AssetManager.h"
#include "Core/InputManager.h"
#include "Core/WorldAssetManager.h"
#include "GameState/IGameState.h"
#include "GameState/MainMenuState.h"
#include "GameState/PauseState.h"
#include "GameState/ServerState.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

GEngine::GEngine(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font)
    : gWindow(window),
      gRenderer(renderer),
      gFont(font),
      assetManager(std::make_unique<AssetManager>(renderer)),
      worldAssetManager(std::make_unique<WorldAssetManager>(renderer)),
      inputManager(std::make_unique<InputManager>(window)) {}

GEngine::~GEngine() {
  while (!gameStates.empty()) {
    PopState();
  }
};

void GEngine::PushState(std::unique_ptr<IGameState> newGameState) {
  if (!newGameState) return;
  newGameState->Init(this);
  gameStates.push_back(std::move(newGameState));
}

void GEngine::PopState() {
  if (gameStates.empty()) return;

  gameStates.back()->Cleanup();
  gameStates.pop_back();
}

void GEngine::ChangeState(std::unique_ptr<IGameState> state) {
  pendingState = std::move(state);
  changeStateRequested = true;
}

void GEngine::Run() {
  using namespace std::chrono;

  steady_clock::time_point startTime;
  steady_clock::time_point curTime;
  steady_clock::time_point prevTime;
  float deltaTime;

  startTime = steady_clock::now();
  curTime = prevTime = startTime;
  // Push the initial state directly instead of using the deferred ChangeState
  PushState(std::make_unique<MainMenuState>());

  while (bIsRunning) {
    curTime = steady_clock::now();
    deltaTime =
        duration<float, milliseconds::period>(curTime - prevTime).count();
    deltaTime /= 1000.f;
    prevTime = curTime;

    inputManager->PrepareForNewFrame();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);  // ImGui gets first look
      inputManager->ProcessEvent(event);  // Our manager gets the event
    }

    if (inputManager->IsQuit()) {
      bIsRunning = false;
    }

    inputManager->UpdateContinuousState();

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (!gameStates.empty()) {
      gameStates.back()->Update(deltaTime);
    }

    // Process deferred state changes at a safe point in the loop
    if (changeStateRequested) {
      if (!gameStates.empty()) {
        PopState();
      }
      PushState(std::move(pendingState));
      changeStateRequested = false;
    }

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), gRenderer);
    SDL_RenderPresent(gRenderer);
  }
}