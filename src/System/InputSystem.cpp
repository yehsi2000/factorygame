#include "System/InputSystem.h"

#include "CommandQueue.h"
#include "Components/TimerComponent.h"
#include "Event.h"
#include "EventDispatcher.h"  // QuitEvent를 위해 EventDispatcher.h가 필요
#include "GEngine.h"
#include "InputState.h"
#include "Registry.h"
#include "Util/TimerUtil.h"
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
  keyBindings[KeyEvent{SDL_SCANCODE_ESCAPE, SDL_KEYDOWN}] = InputAction::Quit;
}

void InputSystem::Update() {
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      engine->GetDispatcher()->Publish(std::make_shared<QuitEvent>());
      return;  // 종료 이벤트 발생 시 추가 입력 처리를 중단
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
      util::AddTimer(engine->GetRegistry()->GetComponent<TimerComponent>(
                         engine->GetPlayer()),
                     TimerId::Interact, 1.f, true);
      break;
    case InputAction::StopInteraction:
      util::RemoveTimer(engine->GetRegistry()->GetComponent<TimerComponent>(
                            engine->GetPlayer()),
                        TimerId::Interact);
      break;
    case InputAction::Quit:
      engine->GetDispatcher()->Publish(std::make_shared<QuitEvent>());
      break;
  }
}

void InputSystem::HandleInputAxis(const Uint8* keyState) {
  if (keyState[SDL_SCANCODE_W]) {
    engine->GetRegistry()->GetInputState().yAxis = -1.f;
  } else if (keyState[SDL_SCANCODE_S]) {
    engine->GetRegistry()->GetInputState().yAxis = 1.f;
  } else {
    engine->GetRegistry()->GetInputState().yAxis = 0.f;
  }

  if (keyState[SDL_SCANCODE_A]) {
    engine->GetRegistry()->GetInputState().xAxis = -1.f;
  } else if (keyState[SDL_SCANCODE_D]) {
    engine->GetRegistry()->GetInputState().xAxis = 1.f;
  } else {
    engine->GetRegistry()->GetInputState().xAxis = 0.f;
  }
}
