#pragma once

#include <vector>

#include "Core/SystemContext.h"
#include "SDL.h"

enum class InputAction {
  StartInteraction,
  StopInteraction,
  Inventory,
  Chat,
  Quit
};

enum class InputType { KEYBOARD, MOUSE };

struct KeyEvent {
  SDL_Scancode Scancode;
  SDL_EventType EventType;
  bool operator==(const KeyEvent& other) const {
    return Scancode == other.Scancode && EventType == other.EventType;
  }
};

class InputSystem {
  std::vector<std::pair<KeyEvent, InputAction>> keyBindings;

  Registry* registry;
  EventDispatcher* eventDispatcher;
  World* world;
  InputManager* inputManager;
  SDL_Window* window;

  SDL_Event event;

  double maxInteractionRadius = 200.0;

 public:
  explicit InputSystem(const SystemContext& context);
  ~InputSystem();
  void Update();

  void HandleInputAction(InputAction action, InputType type);
  void HandleInputAxis(const Uint8* keyState);

 private:
  void RegisterInputBindings();
};
