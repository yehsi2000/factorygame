#ifndef SYSTEM_INPUTSYSTEM_
#define SYSTEM_INPUTSYSTEM_

#include <cstddef>
#include <unordered_map>

#include "CommandQueue.h"
#include "SDL.h"

enum class InputAction {
  StartInteraction,
  StopInteraction,
  Quit
};

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
  SDL_Event event;

 public:
  InputSystem(GEngine* e) : engine(e) {}
  void Update();
  void RegisterInputBindings();
  void HandleInputAction(InputAction action);
  void HandleInputAxis(const Uint8* keyState);
};

#endif /* SYSTEM_INPUTSYSTEM_ */
