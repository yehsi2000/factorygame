#include <atomic>
#include <unordered_map>

#include "CommandQueue.h"
#include "SDL.h"

class Engine;

enum class InputAction {
  StartInteraction,
  StopInteraction,
  MoveUp,
  MoveDown,
  MoveRight,
  MoveLeft,
  Quit
};

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
  Engine* engine;
  CommandQueue* commandQueue;
  std::atomic<bool>& running;
  SDL_Event event;

 public:
  InputSystem(Engine* e, CommandQueue* q, std::atomic<bool>& r)
      : engine(e), commandQueue(q), running(r) {}
  void Update();
  void RegisterInputBindings();
  void HandleInputAction(InputAction action);
  void HandleInputAxis(const Uint8* keyState);
};