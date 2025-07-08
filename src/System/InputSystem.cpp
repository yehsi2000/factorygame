#include "System/InputSystem.h"

#include "CommandQueue.h"
#include "Components/TimerComponent.h"
#include "Event.h"
#include "GEngine.h"
#include "Registry.h"
#include "boost/functional/hash.hpp"

std::size_t KeyEventHasher::operator()(const KeyEvent& k) const {
  using boost::hash_combine;
  using boost::hash_value;

  std::size_t seed = 0;

  hash_combine(seed, hash_value(k.Scancode));
  hash_combine(seed, hash_value(k.EventType));

  // Return the result.
  return seed;
}

void InputSystem::RegisterInputBindings() {
  keyBindings[KeyEvent{SDL_SCANCODE_J, SDL_KEYDOWN}] =
      InputAction::StartInteraction;
  keyBindings[KeyEvent{SDL_SCANCODE_J, SDL_KEYUP}] =
      InputAction::StopInteraction;
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
    if ((event.type == SDL_KEYDOWN && event.key.repeat == 0) ||
        event.type == SDL_KEYUP) {
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
    case InputAction::StartInteraction:
      engine->GetRegistry()->emplaceComponent<TimerComponent>(
          engine->GetPlayer(), 1.f, 1.f, true, [this]() {
            commandQueue->PushEvent(engine->GetDispatcher(),
                                    StartInteractEvent{});
          });
      break;
    case InputAction::StopInteraction:
      engine->GetRegistry()->removeComponent<TimerComponent>(
          engine->GetPlayer());
      commandQueue->PushEvent(engine->GetDispatcher(), StopInteractEvent{});
      break;
    case InputAction::Quit:
      running = false;
      break;
  }
}

void InputSystem::HandleInputAxis(const Uint8* keyState) {
  if (keyState[SDL_SCANCODE_W]) commandQueue->PushEvent(engine->GetDispatcher(), YAxisEvent(-1.f));
  else if (keyState[SDL_SCANCODE_S]) commandQueue->PushEvent(engine->GetDispatcher(), YAxisEvent(1.f));
  else commandQueue->PushEvent(engine->GetDispatcher(), YAxisEvent(0.f));
  
  if (keyState[SDL_SCANCODE_A]) commandQueue->PushEvent(engine->GetDispatcher(), XAxisEvent(-1.f));
  else if (keyState[SDL_SCANCODE_D]) commandQueue->PushEvent(engine->GetDispatcher(), XAxisEvent(1.f));
  else commandQueue->PushEvent(engine->GetDispatcher(), XAxisEvent(0.f));
}
