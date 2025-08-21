#include "System/InputSystem.h"

#include <Components/CameraComponent.h>

#include "Common.h"
#include "Components/AssemblingMachineComponent.h"
#include "Components/BuildingPreviewComponent.h"
#include "Components/InteractionComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TimerComponent.h"
#include "Components/TransformComponent.h"
#include "Core/AssetManager.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/GEngine.h"
#include "Core/InputState.h"
#include "Core/Item.h"
#include "Core/Registry.h"
#include "Core/TileData.h"
#include "Core/TimerManager.h"
#include "Core/World.h"
#include "System/InputSystem.h"
#include "Util/CameraUtil.h"
#include "Util/EntityFactory.h"
#include "Util/MathUtil.h"
#include "Util/TimerUtil.h"
#include "boost/format.hpp"
#include "boost/functional/hash.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_internal.h"
#include <Components/PlayerStateComponent.h>

std::size_t KeyEventHasher::operator()(const KeyEvent& k) const {
  using boost::hash_combine;
  using boost::hash_value;

  std::size_t seed = 0;

  hash_combine(seed, hash_value(k.Scancode));
  hash_combine(seed, hash_value(k.EventType));

  // Return the result.
  return seed;
}

InputSystem::InputSystem(GEngine* e)
    : engine(e),
      io(ImGui::GetIO()),
      isDraggingOutside(false),
      isPreviewingBuilding(false),
      previewingItemID(ItemID::None),
      previewEntity(INVALID_ENTITY) {}

void InputSystem::InitInputSystem() { RegisterInputBindings(); }

void InputSystem::RegisterInputBindings() {
  keyBindings[KeyEvent{SDL_SCANCODE_J, SDL_KEYDOWN}] =
      InputAction::StartInteraction;
  keyBindings[KeyEvent{SDL_SCANCODE_J, SDL_KEYUP}] =
      InputAction::StopInteraction;
  keyBindings[KeyEvent{SDL_SCANCODE_ESCAPE, SDL_KEYDOWN}] = InputAction::Quit;
  keyBindings[KeyEvent{SDL_SCANCODE_I, SDL_KEYDOWN}] = InputAction::Inventory;
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
    if (io.WantCaptureMouse) {
      // Dragging from UI
      ImGuiContext* ctx = ImGui::GetCurrentContext();
      ImGuiWindow* window = ctx->HoveredWindow;
      if (window == nullptr && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        if (!isDraggingOutside) {
          isDraggingOutside = true;
          std::cout << "start dragging outside!" << std::endl;

          // Check if we're dragging a buildable item
          if (ctx->DragDropPayload.DataSize == sizeof(ItemPayload)) {
            ItemPayload* payload_ptr =
                static_cast<ItemPayload*>(ctx->DragDropPayload.Data);
            if (payload_ptr->id != ItemID::None) {
              const ItemDatabase& db = ItemDatabase::instance();
              if (db.IsOfCategory(payload_ptr->id, ItemCategory::Buildable)) {
                isPreviewingBuilding = true;
                previewingItemID = payload_ptr->id;
                CreatePreviewEntity(payload_ptr->id);
              }
            }
          }
        }
        // Update preview entity position while dragging
        if (isPreviewingBuilding && previewEntity != INVALID_ENTITY) {
          UpdatePreviewEntity();
        }
      } else {
        if (isDraggingOutside) {
          isDraggingOutside = false;
          isPreviewingBuilding = false;
          previewingItemID = ItemID::None;
          DestroyPreviewEntity();
        }
      }
      if (isDraggingOutside && event.type == SDL_MOUSEBUTTONUP) {
        if (window == nullptr && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
          HandleInputAction(InputAction::MouseDrop, InputType::MOUSE, ctx);
      }
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
    }
    if (event.type == SDL_MOUSEMOTION) {
      inputState.mousedelta = {event.motion.xrel, event.motion.yrel};
      inputState.mousepos = {event.motion.x, event.motion.y};
    }

    if (!io.WantCaptureKeyboard) {
      auto keystate = SDL_GetKeyboardState(nullptr);
      HandleInputAxis(keystate);
    }

    // if (io.WantCaptureKeyboard) {
    // } else
    if ((event.type == SDL_KEYDOWN && event.key.repeat == 0) ||
        event.type == SDL_KEYUP) {
      auto scancode = event.key.keysym.scancode;
      auto it = keyBindings.find(
          KeyEvent{scancode, static_cast<SDL_EventType>(event.type)});
      if (it != keyBindings.end()) {
        HandleInputAction(it->second, InputType::KEYBOARD);
      }
    }
  }
}

void InputSystem::HandleInputAction(InputAction action, InputType type,
                                    void* params) {
  Registry* registry = engine->GetRegistry();
  registry->GetInputState().xAxis = 0;
  registry->GetInputState().yAxis = 0;

  TimerManager* timerManager = engine->GetTimerManager();
  EntityID player = engine->GetPlayer();

  switch (action) {
    case InputAction::MouseDrop: {
      ImGuiContext* ctx = static_cast<ImGuiContext*>(params);
      if (ctx->DragDropPayload.DataSize == sizeof(ItemPayload)) {
        ItemPayload* payload_ptr =
            static_cast<ItemPayload*>(ctx->DragDropPayload.Data);

        if (payload_ptr->id != ItemID::None && payload_ptr->amount > 0) {
          const ItemDatabase& db = ItemDatabase::instance();

          auto camera = registry->view<CameraComponent>();
          Vec2f worldPos = util::ScreenToWorld(
              registry->GetInputState().mousepos,
              util::GetCameraPosition(registry), engine->GetScreenSize());

          if (db.IsOfCategory(payload_ptr->id, ItemCategory::Buildable)) {
            Vec2 tileIndex =
                engine->GetWorld()->GetTileIndexFromWorldPosition(worldPos);

            EntityID newBuilding = INVALID_ENTITY;
            if (payload_ptr->id == ItemID::AssemblingMachine) {
              if (engine->GetWorld()->CanPlaceBuilding(tileIndex, 2, 2)) {
                newBuilding = factory::CreateAssemblingMachine(
                    *registry, *engine->GetWorld(), engine->GetRenderer(),
                    tileIndex);
              }
            } else if (payload_ptr->id == ItemID::MiningDrill) {
              // TODO: Implement mining drill creation when available
              std::cout << "Mining drill placement not yet implemented"
                        << std::endl;
            }

            if (newBuilding != INVALID_ENTITY) {
              // Successfully placed building, consume the item
              engine->GetDispatcher()->Publish(ItemConsumeEvent{player,payload_ptr->id,1});

              std::cout << "Successfully placed building at " << tileIndex.x
                        << "," << tileIndex.y << std::endl;
            } else {
              std::cout << "Cannot place building at " << tileIndex.x << ","
                        << tileIndex.y << std::endl;
            }
          } else {
            // Handle non-buildable item dropping (create item entity on ground)
            EntityID itemEntity = registry->CreateEntity();

            registry->EmplaceComponent<TransformComponent>(
                itemEntity, TransformComponent{worldPos});

            const ItemData& itemData = db.get(payload_ptr->id);
            SDL_Texture* texture = AssetManager::Instance().getTexture(
                itemData.icon, engine->GetRenderer());

            SpriteComponent sprite;
            sprite.texture = texture;
            sprite.srcRect = {ICONSIZE_SMALL_XSTART, 0, ICONSIZE_SMALL, ICONSIZE_SMALL};  // Assuming 32x32 icons
            sprite.renderRect = {0, 0, ICONSIZE_SMALL, ICONSIZE_SMALL};
            sprite.renderOrder = 0;    // Render above ground
            registry->EmplaceComponent<SpriteComponent>(itemEntity, sprite);

            engine->GetDispatcher()->Publish(ItemConsumeEvent{player,payload_ptr->id,1});

            std::cout << "Dropped " << itemData.name << " at world position "
                      << worldPos.x << "," << worldPos.y << std::endl;

            *payload_ptr = {0, ItemID::None, 0};
          }

          isPreviewingBuilding = false;
          previewingItemID = ItemID::None;
          DestroyPreviewEntity();
        }
      }
    } break;
    case InputAction::StartInteraction:
      if (!registry->HasComponent<InteractionComponent>(player)) {
        const auto& playerTransform =
            registry->GetComponent<TransformComponent>(player);

        Vec2f targetPos;
        InteractionType interactionType = InteractionType::INVALID;

        // Mouse interaction
        if (type == InputType::MOUSE) {
          auto camera = registry->view<CameraComponent>();
          const auto& campos =
              registry->GetComponent<CameraComponent>(camera[0]).position;
          Vec2f mousepos = engine->GetRegistry()->GetInputState().mousepos;
          targetPos =
              util::ScreenToWorld(mousepos, campos, engine->GetScreenSize());
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

        TileData* tiledata =
            engine->GetWorld()->GetTileAtWorldPosition(targetPos);
        if(!tiledata) break;
        
        Vec2 playertileIndex =
            engine->GetWorld()->GetTileIndexFromWorldPosition(
                playerTransform.position);
        
        // registry->AddComponent<InteractionComponent>(
        //     player,
        //     InteractionComponent{player, tileIndex, interactionType, 1.f});
        engine->GetDispatcher()->Publish(PlayerInteractEvent(tiledata->occupyingEntity));
      }
      break;
    case InputAction::StopInteraction:
      if (registry->HasComponent<PlayerStateComponent>(player)) {
        auto& state = registry->GetComponent<PlayerStateComponent>(player);
        if(state.isMining){
          state.isMining = false;
          util::DetachTimer(registry, timerManager, player, TimerId::Mine);
        }
      }
      break;
    case InputAction::Inventory:
      engine->GetDispatcher()->Publish(ToggleInventoryEvent{});
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

void InputSystem::CreatePreviewEntity(ItemID itemID) {
  Registry* registry = engine->GetRegistry();

  // Destroy existing preview entity if it exists
  DestroyPreviewEntity();

  previewEntity = registry->CreateEntity();

  // Determine building size
  int width = 1, height = 1;
  if (itemID == ItemID::AssemblingMachine) {
    width = height = 2;
  } else if (itemID == ItemID::MiningDrill) {
    width = height = 1;
  }

  // Add transform component (position will be updated in UpdatePreviewEntity)
  registry->EmplaceComponent<TransformComponent>(
      previewEntity, TransformComponent{Vec2f(0, 0)});

  // Add preview component
  registry->EmplaceComponent<BuildingPreviewComponent>(
      previewEntity, BuildingPreviewComponent{itemID, width, height});

  // Add sprite component for preview visual
  const ItemDatabase& db = ItemDatabase::instance();
  const ItemData& itemData = db.get(itemID);
  SDL_Texture* texture =
      AssetManager::Instance().getTexture(itemData.icon, engine->GetRenderer());

  if (texture) {
    SpriteComponent sprite;
    sprite.texture = texture;
    sprite.srcRect = {0, 0, ICONSIZE_BIG, ICONSIZE_BIG};
    sprite.renderRect = {0, 0, TILE_PIXEL_SIZE * 2, TILE_PIXEL_SIZE * 2};
    sprite.renderOrder = 1000;  // Render on top
    registry->EmplaceComponent<SpriteComponent>(previewEntity, sprite);
  }
}

void InputSystem::DestroyPreviewEntity() {
  if (previewEntity != INVALID_ENTITY) {
    Registry* registry = engine->GetRegistry();
    registry->DestroyEntity(previewEntity);
    previewEntity = INVALID_ENTITY;
  }
}

void InputSystem::UpdatePreviewEntity() {
  if (previewEntity == INVALID_ENTITY) return;

  Registry* registry = engine->GetRegistry();

  // Get mouse position in world coordinates
  auto camera = registry->view<CameraComponent>();
  if (camera.empty()) return;

  Vec2f mouseWorldPos = util::ScreenToWorld(
      engine->GetRegistry()->GetInputState().mousepos,
      util::GetCameraPosition(registry), engine->GetScreenSize());

  Vec2 tileIndex =
      engine->GetWorld()->GetTileIndexFromWorldPosition(mouseWorldPos);

  // Get preview component to check building size
  auto& previewComp =
      registry->GetComponent<BuildingPreviewComponent>(previewEntity);

  // Check if building can be placed

  // Update position to snap to tile grid
  Vec2f snapWorldPos = (tileIndex * TILE_PIXEL_SIZE);
                      //  +(Vec2{previewComp.width, previewComp.height} / 2);

  std::cout << "snap position : " << snapWorldPos.x << "," << snapWorldPos.y
            << std::endl;

  // Update transform component
  auto& transform = registry->GetComponent<TransformComponent>(previewEntity);
  transform.position = snapWorldPos;
}
