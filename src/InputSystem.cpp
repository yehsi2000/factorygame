#include "InputSystem.h"

#include "Event.h"

void InputSystem::RegisterInputBindings() {
  keyBindings[KeyEvent{SDL_SCANCODE_J, SDL_KEYDOWN}] = InputAction::StartMining;
  keyBindings[KeyEvent{SDL_SCANCODE_J, SDL_KEYUP}] = InputAction::CancelMining;
  keyBindings[KeyEvent{SDL_SCANCODE_W, SDL_KEYDOWN}] = InputAction::MoveUp;
  keyBindings[KeyEvent{SDL_SCANCODE_S, SDL_KEYDOWN}] = InputAction::MoveDown;
  keyBindings[KeyEvent{SDL_SCANCODE_A, SDL_KEYDOWN}] = InputAction::MoveLeft;
  keyBindings[KeyEvent{SDL_SCANCODE_D, SDL_KEYDOWN}] = InputAction::MoveRight;
  keyBindings[KeyEvent{SDL_SCANCODE_ESCAPE, SDL_KEYDOWN}] = InputAction::Quit;
}

void InputSystem::Update() {
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      running = false;
      return;
    }
    if ((event.type == SDL_KEYDOWN && event.key.repeat ==0) || event.type == SDL_KEYUP) {
      auto scancode = event.key.keysym.scancode;
      auto it = keyBindings.find(
          KeyEvent{scancode, static_cast<SDL_EventType>(event.type)});
      if (it != keyBindings.end()) {
        HandleInputAction(it->second);
      }
    }
  }
  auto keystate = SDL_GetKeyboardState(nullptr);
  HandleInputAxis(keystate);
}

void InputSystem::HandleInputAction(InputAction action) {
  switch (action) {
    case InputAction::StartMining:
      commandQueue->PushEvent(world->GetDispatcher(), StartInteractEvent{});
      break;
    case InputAction::CancelMining:
      commandQueue->PushEvent(world->GetDispatcher(), StopInteractEvent{});
      break;
    case InputAction::Quit:
      running = false;
      break;
  }
}

void InputSystem::HandleInputAxis(const Uint8* keyState){
  if(keyState[SDL_SCANCODE_W]) std::cout<<"move up\n";
}