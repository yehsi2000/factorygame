#include "InputSystem.h"

#include "Event.h"

void InputSystem::RegisterInputBindings() {
  keyBindings[KeyEvent{SDL_SCANCODE_J, SDL_KEYDOWN}] = InputAction::StartMining;
  keyBindings[KeyEvent{SDL_SCANCODE_J, SDL_KEYUP}] = InputAction::CancelMining;
  keyBindings[KeyEvent{SDL_SCANCODE_W, SDL_KEYDOWN}] = InputAction::MoveUp;
  keyBindings[KeyEvent{SDL_SCANCODE_S, SDL_KEYDOWN}] = InputAction::MoveDown;
  keyBindings[KeyEvent{SDL_SCANCODE_A, SDL_KEYDOWN}] = InputAction::MoveLeft;
  keyBindings[KeyEvent{SDL_SCANCODE_D, SDL_KEYDOWN}] = InputAction::MoveRight;
}

void InputSystem::Update() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
      auto scancode = event.key.keysym.scancode;
      auto it = keyBindings.find(
          KeyEvent{scancode, static_cast<SDL_EventType>(event.type)});
      if (it != keyBindings.end()) {
        HandleInputAction(it->second);
      }
    }
  }
}

void InputSystem::HandleInputAction(InputAction action) {
  switch (action) {
    case InputAction::StartMining:
      commandQueue->PushEvent(world->GetDispatcher(),StartMiningEvent{});
      break;
    case InputAction::CancelMining:
      commandQueue->PushEvent(world->GetDispatcher(),StopMiningEvent{});
      break;
    case InputAction::MoveUp:
      // world->GetPlayer()->Move(0, -1);
      break;
  }
}