#include "GameState/MainMenuState.h"

#include "Core/GEngine.h"
#include "GameState/ServerState.h"
#include "GameState/ClientState.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"


void MainMenuState::Init(GEngine* engine) {
  gEngine = engine;
  gWindow = engine->GetWindow();
  gRenderer = engine->GetRenderer();
  gFont = engine->GetFont();
}

void MainMenuState::Cleanup() {}

void MainMenuState::Update(float deltaTime) {
  // Set up a full-screen, unmovable, borderless window
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

  ImGui::Begin("Main Menu", nullptr, window_flags);

  ImGui::SetCursorPosY(viewport->WorkSize.y/2 - 75.0f);
  float windowWidth = ImGui::GetWindowSize().x;
  float buttonWidth = 200.0f;
  ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

  if (ImGui::Button("Start Server Game", ImVec2(buttonWidth, 50))) {
    ImGui::End();
    gEngine->ChangeState(std::make_unique<ServerState>());
    return;
  }

  ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
  if (ImGui::Button("Connect To Server", ImVec2(buttonWidth, 50))) {
    ImGui::End();
    std::unique_ptr<ClientState> clientState = std::make_unique<ClientState>();
    if(clientState->TryConnect())
      gEngine->ChangeState(std::unique_ptr<IGameState>(std::move(clientState)));
    return;
  }

  ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
  if (ImGui::Button("Quit", ImVec2(buttonWidth, 50))) {
    gEngine->Stop();
  }

  ImGui::End();
}