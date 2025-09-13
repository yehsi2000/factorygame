#include "Core/InputManager.h"

#include <cstring>

#include "Core/InputManager.h"
#include "SDL.h"
#include "imgui.h"
#include "imgui_internal.h"

InputManager::InputManager(SDL_Window* window) : window(window), io(ImGui::GetIO()) {
  currentKeyState = SDL_GetKeyboardState(nullptr);
  SDL_GetKeyboardState(&numKeys);
  prevKeyState.resize(numKeys);

  std::memcpy(prevKeyState.data(), currentKeyState, numKeys);
}

InputManager::~InputManager() = default;

void InputManager::PrepareForNewFrame() {
  state.bIsLeftMousePressed = false;
  state.bIsRightMousePressed = false;
  state.bIsLeftMouseReleased = false;
  state.bIsRightMouseReleased = false;
  state.mouseWheel = {0, 0};
  state.mouseDelta = {0, 0};

  std::memcpy(prevKeyState.data(), currentKeyState, numKeys);
  currentKeyState = SDL_GetKeyboardState(nullptr);
}

void InputManager::ProcessEvent(const SDL_Event& event) {
  if (event.type == SDL_QUIT) {
    state.bIsQuit = true;
  }

  // This is the new, more robust condition
  bool bIsUIBusy = io.WantCaptureMouse || ImGui::IsAnyItemActive();

  switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
      if (bIsUIBusy) break;
      if (event.button.button == SDL_BUTTON_LEFT) {
        state.bIsLeftMouseDown = true;
        state.bIsLeftMousePressed = true;
      } else if (event.button.button == SDL_BUTTON_RIGHT) {
        state.bIsRightMouseDown = true;
        state.bIsRightMousePressed = true;
      }
      break;

    case SDL_MOUSEBUTTONUP:
      if (bIsUIBusy) break;
      if (event.button.button == SDL_BUTTON_LEFT) {
        state.bIsLeftMouseDown = false;
        state.bIsLeftMouseReleased = true;
      } else if (event.button.button == SDL_BUTTON_RIGHT) {
        state.bIsRightMouseDown = false;
        state.bIsRightMouseReleased = true;
      }
      break;

    case SDL_MOUSEWHEEL:
      if (bIsUIBusy) break;
      state.mouseWheel = {event.wheel.x, event.wheel.y};
      break;

    case SDL_MOUSEMOTION:
      state.mousePos = {event.motion.x, event.motion.y};
      if (bIsUIBusy) break;
      state.mouseDelta = {event.motion.xrel, event.motion.yrel};
      break;
  }
}

void InputManager::UpdateContinuousState() {
  if (io.WantCaptureKeyboard) {
    state.axis = {0, 0};
    return;
  }

  state.axis.x = static_cast<float>(currentKeyState[SDL_SCANCODE_D] -
                                    currentKeyState[SDL_SCANCODE_A]);
  state.axis.y = static_cast<float>(currentKeyState[SDL_SCANCODE_S] -
                                    currentKeyState[SDL_SCANCODE_W]);
}

bool InputManager::IsKeyDown(SDL_Scancode key) const {
  if (io.WantCaptureKeyboard) return false;
  return currentKeyState[key];
}

bool InputManager::IsKeyUp(SDL_Scancode key) const {
  if (io.WantCaptureKeyboard) return true;
  return !currentKeyState[key];
}

bool InputManager::WasKeyPressedThisFrame(SDL_Scancode key) const {
  if (io.WantCaptureKeyboard) return false;
  return currentKeyState[key] && !prevKeyState[key];
}

bool InputManager::WasKeyReleasedThisFrame(SDL_Scancode key) const {
  if (io.WantCaptureKeyboard) return false;
  return !currentKeyState[key] && prevKeyState[key];
}

bool InputManager::IsMouseButtonDown(MouseButton button) const {
  if (io.WantCaptureMouse) return false;
  if (button == MouseButton::LEFT) return state.bIsLeftMouseDown;
  if (button == MouseButton::RIGHT) return state.bIsRightMouseDown;
  return false;
}

bool InputManager::IsMouseButtonUp(MouseButton button) const {
  return !IsMouseButtonDown(button);
}

bool InputManager::WasMouseButtonPressed(MouseButton button) const {
  if (io.WantCaptureMouse) return false;
  if (button == MouseButton::LEFT) return state.bIsLeftMousePressed;
  if (button == MouseButton::RIGHT) return state.bIsRightMousePressed;
  return false;
}

bool InputManager::WasMouseButtonReleased(MouseButton button) const {
  if (io.WantCaptureMouse) return false;
  if (button == MouseButton::LEFT) return state.bIsLeftMouseReleased;
  if (button == MouseButton::RIGHT) return state.bIsRightMouseReleased;
  return false;
}

Vec2 InputManager::GetMousePosition() const { return state.mousePos; }

Vec2 InputManager::GetMouseDelta() const { return state.mouseDelta; }

int InputManager::GetMouseWheelScroll() const { return state.mouseWheel.y; }

Vec2 InputManager::GetAxis() const { return state.axis; }

int InputManager::GetXAxis() const { return state.axis.x; }

int InputManager::GetYAxis() const { return state.axis.y; }

Vec2 InputManager::GetScreenSize() {
  SDL_GetWindowSize(window, &screenSize.x, &screenSize.y);
  return screenSize;
}

bool InputManager::IsQuit() const { return state.bIsQuit; }
