#pragma once

#include <unordered_map>

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
  MoveLeft
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

 public:
  InputSystem(World* w, CommandQueue* q) : world(w), commandQueue(q) {}
  void Update();
  void RegisterInputBindings();
  void HandleInputAction(InputAction action);
};