#include "Core/InputManager.h"

#include <cstring>  // For memcpy

#include "Core/InputManager.h"
#include "SDL.h"
#include "imgui.h"
#include "imgui_internal.h"

InputManager::InputManager(SDL_Window* window) : window(window), io(ImGui::GetIO()) {
  currentKeyState = SDL_GetKeyboardState(nullptr);
  SDL_GetKeyboardState(&numKeys);
  prevKeyState.resize(numKeys);

  memcpy(prevKeyState.data(), currentKeyState, numKeys);
}

InputManager::~InputManager() = default;

void InputManager::PrepareForNewFrame() {
  state.leftMousePressed = false;
  state.rightMousePressed = false;
  state.leftMouseReleased = false;
  state.rightMouseReleased = false;
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
  bool UIBusy = io.WantCaptureMouse || ImGui::IsAnyItemActive();

  switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
      if (UIBusy) break;
      if (event.button.button == SDL_BUTTON_LEFT) {
        state.leftMouseDown = true;
        state.leftMousePressed = true;
      } else if (event.button.button == SDL_BUTTON_RIGHT) {
        state.rightMouseDown = true;
        state.rightMousePressed = true;
      }
      break;

    case SDL_MOUSEBUTTONUP:
      if (UIBusy) break;
      if (event.button.button == SDL_BUTTON_LEFT) {
        state.leftMouseDown = false;
        state.leftMouseReleased = true;
      } else if (event.button.button == SDL_BUTTON_RIGHT) {
        state.rightMouseDown = false;
        state.rightMouseReleased = true;
      }
      break;

    case SDL_MOUSEWHEEL:
      if (UIBusy) break;
      state.mouseWheel = {event.wheel.x, event.wheel.y};
      break;

    case SDL_MOUSEMOTION:
      state.mousePos = {event.motion.x, event.motion.y};
      if (UIBusy) break;
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
  if (button == MouseButton::LEFT) return state.leftMouseDown;
  if (button == MouseButton::RIGHT) return state.rightMouseDown;
  return false;
}

bool InputManager::IsMouseButtonUp(MouseButton button) const {
  return !IsMouseButtonDown(button);
}

bool InputManager::WasMouseButtonPressed(MouseButton button) const {
  if (io.WantCaptureMouse) return false;
  if (button == MouseButton::LEFT) return state.leftMousePressed;
  if (button == MouseButton::RIGHT) return state.rightMousePressed;
  return false;
}

bool InputManager::WasMouseButtonReleased(MouseButton button) const {
  if (io.WantCaptureMouse) return false;
  if (button == MouseButton::LEFT) return state.leftMouseReleased;
  if (button == MouseButton::RIGHT) return state.rightMouseReleased;
  return false;
}

// bool InputManager::IsUIMouseButtonDown(MouseButton button) const {
//   if (!io.WantCaptureMouse) return false;
//   if (button == MouseButton::LEFT)
//     return io.MouseDown[0];
//   else if (button == MouseButton::RIGHT)
//     return io.MouseDown[1];
//   return false;
// }

// bool InputManager::IsUIMouseButtonUp(MouseButton button) const {
//   if (!io.WantCaptureMouse) return false;
//   if (button == MouseButton::LEFT)
//     return !io.MouseDown[0];
//   else if (button == MouseButton::RIGHT)
//     return !io.MouseDown[1];
//   return false;
// }

// bool InputManager::WasUIMouseButtonPressed(MouseButton button) const {
//   if (!io.WantCaptureMouse) return false;
//   if (button == MouseButton::LEFT)
//     return io.MouseClicked[0];
//   else if (button == MouseButton::RIGHT)
//     return io.MouseClicked[1];
//   return false;
// }

// bool InputManager::WasUIMouseButtonReleased(MouseButton button) const {
//   if (!io.WantCaptureMouse) return false;
//   if (button == MouseButton::LEFT)
//     return io.MouseReleased[0];
//   else if (button == MouseButton::RIGHT)
//     return io.MouseReleased[1];
//   return false;
// }



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
