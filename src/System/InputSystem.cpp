#include "System/InputSystem.h"

#include "Components/TimerComponent.h"
#include "Core/CommandQueue.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/GEngine.h"
#include "Core/InputState.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"
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
  // Reset frame-specific mouse states
  auto& inputState = engine->GetRegistry()->GetInputState();
  inputState.rightMousePressed = false;
  inputState.rightMouseReleased = false;
  inputState.mouseDeltaX = 0;
  inputState.mouseDeltaY = 0;

  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      engine->GetDispatcher()->Publish(QuitEvent{});
      return;  // 종료 이벤트 발생 시 추가 입력 처리를 중단
    }

    // Handle mouse input
    if (event.type == SDL_MOUSEBUTTONDOWN) {
      if (event.button.button == SDL_BUTTON_RIGHT) {
        inputState.rightMouseDown = true;
        inputState.rightMousePressed = true;
      }
    } else if (event.type == SDL_MOUSEBUTTONUP) {
      if (event.button.button == SDL_BUTTON_RIGHT) {
        inputState.rightMouseDown = false;
        inputState.rightMouseReleased = true;
      }
    } else if (event.type == SDL_MOUSEMOTION) {
      inputState.mouseDeltaX = event.motion.xrel;
      inputState.mouseDeltaY = event.motion.yrel;
      inputState.mouseX = event.motion.x;
      inputState.mouseY = event.motion.y;
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
  Registry& registry = *engine->GetRegistry();
  TimerManager& timerManager = *engine->GetTimerManager();
  EntityID player = engine->GetPlayer();

  switch (action) {
    case InputAction::StartInteraction:
      // Use the new, clean utility function to attach a timer.
      util::AttachTimer(registry, timerManager, player, TimerId::Interact, 1.f,
                        true);
      break;
    case InputAction::StopInteraction:
      // Use the new, clean utility function to detach a timer.
      util::DetachTimer(registry, timerManager, player, TimerId::Interact);
      break;
    case InputAction::Quit:
      engine->GetDispatcher()->Publish(QuitEvent{});
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
