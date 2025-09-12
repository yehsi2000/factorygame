#include "System/InputSystem.h"

#include <optional>

#include "Common.h"
#include "Components/AnimationComponent.h"
#include "Components/BuildingPreviewComponent.h"
#include "Components/CameraComponent.h"
#include "Components/InactiveComponent.h"
#include "Components/PlayerStateComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TimerComponent.h"
#include "Components/TransformComponent.h"
#include "Core/AssetManager.h"
#include "Core/EntityFactory.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/GEngine.h"
#include "Core/InputManager.h"
#include "Core/Item.h"
#include "Core/Registry.h"
#include "Core/TileData.h"
#include "Core/World.h"
#include "SDL_events.h"
#include "Util/AnimUtil.h"
#include "Util/CameraUtil.h"
#include "Util/MathUtil.h"
#include "Util/TimerUtil.h"
#include "boost/functional/hash.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_internal.h"

InputSystem::InputSystem(const SystemContext &context)
    : registry(context.registry),
      eventDispatcher(context.eventDispatcher),
      world(context.world),
      inputManager(context.inputManager) {
  RegisterInputBindings();
}

void InputSystem::RegisterInputBindings() {
  keyBindings.push_back(
      {{SDL_SCANCODE_J, SDL_KEYDOWN}, InputAction::StartInteraction});
  keyBindings.push_back(
      {{SDL_SCANCODE_J, SDL_KEYUP}, InputAction::StopInteraction});
  keyBindings.push_back(
      {{SDL_SCANCODE_ESCAPE, SDL_KEYDOWN}, InputAction::Quit});
  keyBindings.push_back(
      {{SDL_SCANCODE_I, SDL_KEYDOWN}, InputAction::Inventory});
  keyBindings.push_back(
      {{SDL_SCANCODE_RETURN, SDL_KEYDOWN}, InputAction::Chat});
}

void InputSystem::Update() {
  // Reset frame-specific mouse states
  if (inputManager->IsQuit()) {
    HandleInputAction(InputAction::Quit, InputType::KEYBOARD);
  }

  if (inputManager->WasMouseButtonPressed(MouseButton::LEFT)) {
    HandleInputAction(InputAction::StartInteraction, InputType::MOUSE);

  } else if (inputManager->WasMouseButtonReleased(MouseButton::LEFT)) {
    HandleInputAction(InputAction::StopInteraction, InputType::MOUSE);
  }

  for (auto &[key, action] : keyBindings) {
    if (inputManager->WasKeyPressedThisFrame(key.Scancode) &&
        key.EventType == SDL_KEYDOWN) {
      HandleInputAction(action, InputType::KEYBOARD);
    } else if (inputManager->WasKeyReleasedThisFrame(key.Scancode) &&
               key.EventType == SDL_KEYUP) {
      HandleInputAction(action, InputType::KEYBOARD);
    }
  }
}

void InputSystem::HandleInputAction(InputAction action, InputType type) {
  EntityID player = world->GetPlayer();

  switch (action) {
    // Player started interacting
    case InputAction::StartInteraction: {
      Vec2f targetPos;
      if (type == InputType::MOUSE) {
        const Vec2f campos = util::GetCameraPosition(registry);
        const Vec2f mousepos = inputManager->GetMousePosition();
        targetPos = util::ScreenToWorld(mousepos, campos,
                                        inputManager->GetScreenSize());
      } else if (type == InputType::KEYBOARD) {
        auto &playerTransform =
            registry->GetComponent<TransformComponent>(player);
        targetPos = playerTransform.position;
      }
      eventDispatcher->Publish(PlayerInteractEvent(targetPos));
      break;
    }

    // Player ended interacting
    case InputAction::StopInteraction: {
      eventDispatcher->Publish(PlayerEndInteractEvent{});
      break;
    }

    // Open Inventory
    case InputAction::Inventory:
      eventDispatcher->Publish(ToggleInventoryEvent{});
      break;

    case InputAction::Chat:
      eventDispatcher->Publish(ToggleChatInputEvent{});
      break;
    // Quit game
    case InputAction::Quit:
      eventDispatcher->Publish(QuitEvent{});
      break;
  }
}

InputSystem::~InputSystem() = default;
