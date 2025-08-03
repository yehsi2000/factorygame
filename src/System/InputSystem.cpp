#include "System/InputSystem.h"

#include <Components/CameraComponent.h>

#include <format>

#include "Components/InteractionComponent.h"
#include "Components/TimerComponent.h"
#include "Core/CommandQueue.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/GEngine.h"
#include "Core/InputState.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"
#include "SDL.h"
#include "Util/MathUtil.h"
#include "Util/TimerUtil.h"
#include "boost/functional/hash.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

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
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);

    if (event.type == SDL_QUIT) {
      engine->GetDispatcher()->Publish(QuitEvent{});
      return;  // Stop all input handling if quit event occurs
    }
    if (engine->GetGuiIO().WantCaptureMouse) {
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
      if (event.button.button == SDL_BUTTON_LEFT) {
        HandleInputAction(InputAction::StartInteraction, InputType::MOUSE);
      } else {
        inputState.rightMouseDown = true;
        inputState.rightMousePressed = true;
      }
    } else if (event.type == SDL_MOUSEBUTTONUP) {
      if (event.button.button == SDL_BUTTON_LEFT) {
        HandleInputAction(InputAction::StopInteraction, InputType::MOUSE);
      } else {
        inputState.rightMouseDown = false;
        inputState.rightMouseReleased = true;
      }
    } else if (event.type == SDL_MOUSEMOTION) {
      inputState.mouseDeltaX = event.motion.xrel;
      inputState.mouseDeltaY = event.motion.yrel;
      inputState.mouseX = event.motion.x;
      inputState.mouseY = event.motion.y;
      // Registry* registry = engine->GetRegistry();
      // EntityID player = engine->GetPlayer();
      // if (registry->HasComponent<InteractionComponent>(player)) {
      //   if (registry->GetComponent<InteractionComponent>(player).type ==
      //       InteractionType::MOUSE) {
      //     registry->RemoveComponent<InteractionComponent>(player);
      //     TimerManager* timerManager = engine->GetTimerManager();
      //     util::DetachTimer(*registry, *timerManager, player,
      //                       TimerId::Interact);
      //   }
      // }
    }
    if (engine->GetGuiIO().WantCaptureKeyboard) {
    } else if ((event.type == SDL_KEYDOWN && event.key.repeat == 0) ||
               event.type == SDL_KEYUP) {
      auto scancode = event.key.keysym.scancode;
      auto it = keyBindings.find(
          KeyEvent{scancode, static_cast<SDL_EventType>(event.type)});
      if (it != keyBindings.end()) {
        HandleInputAction(it->second, InputType::KEYBOARD);
      }
    }
  }
  auto keystate = SDL_GetKeyboardState(nullptr);
  HandleInputAxis(keystate);
}

void InputSystem::HandleInputAction(InputAction action, InputType type) {
  Registry& registry = *engine->GetRegistry();
  TimerManager& timerManager = *engine->GetTimerManager();
  EntityID player = engine->GetPlayer();

  switch (action) {
    case InputAction::StartInteraction:
      if (!registry.HasComponent<InteractionComponent>(player)) {
        const auto& playerTransform =
            registry.GetComponent<TransformComponent>(player);

        Vec2f targetPos;
        InteractionType interactionType = InteractionType::INVALID;

        // Mouse interaction
        if (type == InputType::MOUSE) {
          auto camera = registry.view<CameraComponent>();
          int screenWidth, screenHeight;
          SDL_GetRendererOutputSize(engine->GetRenderer(), &screenWidth,
                                    &screenHeight);
          const auto& campos =
              registry.GetComponent<CameraComponent>(camera[0]).position;
          targetPos = Vec2f(engine->GetRegistry()->GetInputState().mouseX +
                                campos.x - (screenWidth / 2.f),
                            engine->GetRegistry()->GetInputState().mouseY +
                                campos.y - (screenHeight / 2.f));
          auto dist = util::dist(playerTransform.position, targetPos);
          if (maxInteractionRadius < dist) break;
          interactionType = InteractionType::MOUSE;
        }

        // Keyboard interaction
        else if (type == InputType::KEYBOARD) {
          targetPos =
              Vec2f(playerTransform.position.x, playerTransform.position.y);
          interactionType = InteractionType::KEYBOARD;
        }

        if (interactionType == InteractionType::INVALID) break;

        Vec2 tilecoord =
            engine->GetWorld()->GetTileCoordFromWorldPosition(targetPos);

        Vec2 playertilecoord =
            engine->GetWorld()->GetTileCoordFromWorldPosition(
                playerTransform.position);

        std::cout << std::format(
            "targettile = {}:{}, playertile = {}:{} type : {}\n", tilecoord.x,
            tilecoord.y, playertilecoord.x, playertilecoord.y,
            (interactionType == InteractionType::KEYBOARD ? "keyboard"
                                                          : "mouse"));

        registry.AddComponent<InteractionComponent>(
            player,
            InteractionComponent{player, tilecoord, interactionType, 1.f});

        util::AttachTimer(registry, timerManager, player, TimerId::Interact,
                          1.f, true);
      }
      break;
    case InputAction::StopInteraction:
      if (registry.HasComponent<InteractionComponent>(player)) {
        registry.RemoveComponent<InteractionComponent>(player);
        util::DetachTimer(registry, timerManager, player, TimerId::Interact);
      }
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
