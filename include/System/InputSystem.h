#ifndef SYSTEM_INPUTSYSTEM_
#define SYSTEM_INPUTSYSTEM_

#include <cstddef>
#include <unordered_map>

#include "Core/CommandQueue.h"
#include "SDL.h"
#include "imgui.h"

enum class InputAction { StartInteraction, StopInteraction, Inventory, Quit };
enum class InputType { KEYBOARD, MOUSE };

class GEngine;

struct KeyEvent {
  SDL_Scancode Scancode;
  SDL_EventType EventType;
  bool operator==(const KeyEvent& other) const {
    return Scancode == other.Scancode && EventType == other.EventType;
  }
};

struct KeyEventHasher {
  std::size_t operator()(const KeyEvent& k) const;
};

class InputSystem {
  std::unordered_map<KeyEvent, InputAction, KeyEventHasher> keyBindings;
  GEngine* engine;
  ImGuiIO& io;
  double maxInteractionRadius = 3000.0;

 public:
  InputSystem(GEngine* e);
  void Update();
  void RegisterInputBindings();
  void HandleInputAction(InputAction action, InputType type);
  void HandleInputAxis(const Uint8* keyState);
};

#endif /* SYSTEM_INPUTSYSTEM_ */
