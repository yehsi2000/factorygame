#include "Core/InputManager.h"

#include <cstring>

#include "Core/InputManager.h"
#include "imgui.h"

InputManager::InputManager(SDL_Window* window) : window(window), io(ImGui::GetIO()) {
  currentKeyState = SDL_GetKeyboardState(nullptr);
  SDL_GetKeyboardState(&numKeys);
  prevKeyState.resize(numKeys);

  std::memcpy(prevKeyState.data(), currentKeyState, numKeys);
}

InputManager::~InputManager() = default;

void InputManager::PrepareForNewFrame() {
  state.isLeftMousePressed = false;
  state.isRightMousePressed = false;
  state.isLeftMouseReleased = false;
  state.isRightMouseReleased = false;
  state.mouseWheel = {0, 0};
  state.mouseDelta = {0, 0};

  std::memcpy(prevKeyState.data(), currentKeyState, numKeys);
  currentKeyState = SDL_GetKeyboardState(nullptr);
}

void InputManager::ProcessEvent(const SDL_Event& event) {
  if (event.type == SDL_QUIT) {
    state.isQuit = true;
  }

  // This is the new, more robust condition
  bool isUIBusy = io.WantCaptureMouse || ImGui::IsAnyItemActive();

  switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
      if (isUIBusy) break;
      if (event.button.button == SDL_BUTTON_LEFT) {
        state.isLeftMouseDown = true;
        state.isLeftMousePressed = true;
      } else if (event.button.button == SDL_BUTTON_RIGHT) {
        state.isRightMouseDown = true;
        state.isRightMousePressed = true;
      }
      break;

    case SDL_MOUSEBUTTONUP:
      if (isUIBusy) break;
      if (event.button.button == SDL_BUTTON_LEFT) {
        state.isLeftMouseDown = false;
        state.isLeftMouseReleased = true;
      } else if (event.button.button == SDL_BUTTON_RIGHT) {
        state.isRightMouseDown = false;
        state.isRightMouseReleased = true;
      }
      break;

    case SDL_MOUSEWHEEL:
      if (isUIBusy) break;
      state.mouseWheel = {event.wheel.x, event.wheel.y};
      break;

    case SDL_MOUSEMOTION:
      state.mousePos = {event.motion.x, event.motion.y};
      if (isUIBusy) break;
      state.mouseDelta = {event.motion.xrel, event.motion.yrel};
      break;
  }
}

void InputManager::UpdateContinuousState() {
  if (io.WantCaptureKeyboard) {
    state.axis = {0, 0};
    return;
  }

  state.axis.x =
      currentKeyState[SDL_SCANCODE_D] - currentKeyState[SDL_SCANCODE_A];
  state.axis.y =
      currentKeyState[SDL_SCANCODE_S] - currentKeyState[SDL_SCANCODE_W];
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
  if (button == MouseButton::LEFT) return state.isLeftMouseDown;
  if (button == MouseButton::RIGHT) return state.isRightMouseDown;
  return false;
}

bool InputManager::IsMouseButtonUp(MouseButton button) const {
  return !IsMouseButtonDown(button);
}

bool InputManager::WasMouseButtonPressed(MouseButton button) const {
  if (io.WantCaptureMouse) return false;
  if (button == MouseButton::LEFT) return state.isLeftMousePressed;
  if (button == MouseButton::RIGHT) return state.isRightMousePressed;
  return false;
}

bool InputManager::WasMouseButtonReleased(MouseButton button) const {
  if (io.WantCaptureMouse) return false;
  if (button == MouseButton::LEFT) return state.isLeftMouseReleased;
  if (button == MouseButton::RIGHT) return state.isRightMouseReleased;
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

bool InputManager::IsQuit() const { return state.isQuit; }
