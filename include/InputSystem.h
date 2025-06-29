#pragma once

#include <unordered_map>
#include <atomic>

#include "SDL.h"
#include "World.h"
#include "CommandQueue.h"
#include "boost/functional/hash.hpp"

enum class InputAction {
  StartMining,
  CancelMining,
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
  std::size_t operator()(const KeyEvent& k) const {
    using boost::hash_combine;
    using boost::hash_value;

    std::size_t seed = 0;

    hash_combine(seed, hash_value(k.Scancode));
    hash_combine(seed, hash_value(k.EventType));

    // Return the result.
    return seed;
  }
};

class InputSystem {
  std::unordered_map<KeyEvent, InputAction, KeyEventHasher> keyBindings;
  World* world;
  CommandQueue* commandQueue;
  std::atomic<bool>& running;
  SDL_Event event;

 public:
  InputSystem(World* w, CommandQueue* q, std::atomic<bool>& r) : world(w), commandQueue(q), running(r) {}
  void Update();
  void RegisterInputBindings();
  void HandleInputAction(InputAction action);
  void HandleInputAxis(const Uint8* keyState);
};