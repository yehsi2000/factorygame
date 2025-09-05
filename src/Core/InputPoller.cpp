#include "Core/InputPoller.h"

#include <cstring>

#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "SDL_events.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_internal.h"

InputPoller::InputPoller(SDL_Window *window)
    : io(ImGui::GetIO()), window(window) {
  std::memset(&state, 0, sizeof(state));

  keyState = SDL_GetKeyboardState(&numKeys);

  prevKeyState.resize(SDL_NUM_SCANCODES);

  std::memcpy(prevKeyState.data(), keyState, numKeys);

  SDL_GetWindowSize(window, &screenSize.x, &screenSize.y);
}

void InputPoller::PollEvents() {
  // Reset per-frame state for mouse buttons and deltas
  state.leftMousePressed = false;
  state.rightMousePressed = false;
  state.leftMouseReleased = false;
  state.rightMouseReleased = false;
  state.mouseWheel = {0, 0};
  state.mouseDelta = {0, 0};

  // Update keyboard state snapshot once per frame, before processing events
  std::memcpy(prevKeyState.data(), keyState, numKeys);
  keyState = SDL_GetKeyboardState(nullptr);

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT) {
      state.isQuit = true;
    }

    if (io.WantCaptureMouse) {
      ImGuiContext *ctx = ImGui::GetCurrentContext();
      ImGuiWindow *window = ctx->HoveredWindow;

      if (window == nullptr && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        state.isDraggingOutside = true;
      } else {
        state.isDraggingOutside = false;
      }
    } else {
      state.isDraggingOutside = false;
    }
    // Handle game-world events only if ImGui doesn't want the mouse
    switch (event.type) {
      case SDL_MOUSEBUTTONDOWN: {
        if (event.button.button == SDL_BUTTON_LEFT) {
          state.leftMouseDown = true;
          state.leftMousePressed = true;
        } else if (event.button.button == SDL_BUTTON_RIGHT) {
          state.rightMouseDown = true;
          state.rightMousePressed = true;
        }
        break;
      }

      case SDL_MOUSEBUTTONUP: {
        if (event.button.button == SDL_BUTTON_LEFT) {
          state.leftMouseDown = false;
          state.leftMouseReleased = true;
        } else if (event.button.button == SDL_BUTTON_RIGHT) {
          state.rightMouseDown = false;
          state.rightMouseReleased = true;
        }
        break;
      }

      case SDL_MOUSEWHEEL: {
        state.mouseWheel = {event.wheel.x, event.wheel.y};
        break;
      }

      case SDL_MOUSEMOTION: {
        state.mouseDelta = {event.motion.xrel, event.motion.yrel};
        state.mousePos = {event.motion.x, event.motion.y};
        break;
      }
      default:
        break;
    }
  }

  // Update continuous axis states from the keyboard snapshot.
  // The InputSystem is responsible for checking io.WantCaptureKeyboard before
  // using these.
  state.axis.x = keyState[SDL_SCANCODE_D] - keyState[SDL_SCANCODE_A];
  state.axis.y = keyState[SDL_SCANCODE_S] - keyState[SDL_SCANCODE_W];
}

bool InputPoller::IsKeyDown(SDL_Scancode key) const { return keyState[key]; }

bool InputPoller::IsKeyUp(SDL_Scancode key) const { return !keyState[key]; }

bool InputPoller::WasKeyPressedThisFrame(SDL_Scancode key) const {
  return keyState[key] && !prevKeyState[key];
}

bool InputPoller::WasKeyReleasedThisFrame(SDL_Scancode key) const {
  return !keyState[key] && prevKeyState[key];
}

bool InputPoller::IsMouseButtonDown(Mouse button) const {
  if (Mouse::LEFT == button)
    return state.leftMouseDown;
  else  // if (Mouse::RIGHT == button)
    return state.rightMouseDown;
}

bool InputPoller::IsMouseButtonUp(Mouse button) const {
  if (Mouse::LEFT == button)
    return !state.leftMouseDown;
  else  // if (Mouse::RIGHT == button)
    return !state.rightMouseDown;
}

bool InputPoller::WasMouseButtonPressed(Mouse button) const {
  if (Mouse::LEFT == button)
    return state.leftMousePressed;
  else  // if (Mouse::RIGHT == button)
    return state.rightMousePressed;
}

bool InputPoller::WasMouseButtonReleased(Mouse button) const {
  if (Mouse::LEFT == button)
    return state.leftMouseReleased;
  else  // if (Mouse::RIGHT == button)
    return state.rightMouseReleased;
}

int InputPoller::GetScrollAmount() { return state.mouseWheel.y; }

Vec2 InputPoller::GetMousePositon() const { return state.mousePos; }

InputPoller::~InputPoller() = default;