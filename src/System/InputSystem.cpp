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
#include "Core/InputState.h"
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

std::size_t KeyEventHasher::operator()(const KeyEvent &k) const {
  using boost::hash_combine;
  using boost::hash_value;

  std::size_t seed = 0;

  hash_combine(seed, hash_value(k.Scancode));
  hash_combine(seed, hash_value(k.EventType));

  // Return the result.
  return seed;
}

InputSystem::InputSystem(const SystemContext &context, SDL_Window *window)
    : registry(context.registry),
      eventDispatcher(context.eventDispatcher),
      world(context.world),
      factory(context.entityFactory),
      timerManager(context.timerManager),
      assetManager(context.assetManager),
      window(window),
      io(ImGui::GetIO()),
      isDraggingOutside(false),
      isPreviewingBuilding(false),
      previewingItemID(ItemID::None),
      previewEntity(INVALID_ENTITY) {
  RegisterInputBindings();
}

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
  auto &inputState = registry->GetInputState();
  inputState.rightMousePressed = false;
  inputState.rightMouseReleased = false;
  inputState.mouseDeltaX = 0;
  inputState.mouseDeltaY = 0;
  SDL_GetWindowSize(window, &screenSize.x, &screenSize.y);

  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);

    if (event.type == SDL_QUIT) {
      eventDispatcher->Publish(QuitEvent{});
      return;  // Stop all input handling if quit event occurs
    }

    // Handle Mouse Event
    if (io.WantCaptureMouse) {
      // Dragging from UI
      ImGuiContext *ctx = ImGui::GetCurrentContext();
      ImGuiWindow *window = ctx->HoveredWindow;

      if (window == nullptr && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        if (!isDraggingOutside) {
          isDraggingOutside = true;

          // Handle item dragging
          if (ctx->DragDropPayload.DataSize == sizeof(ItemPayload)) {
            ItemPayload *payload_ptr =
                static_cast<ItemPayload *>(ctx->DragDropPayload.Data);

            // Only handle dragging from player inventory
            if (payload_ptr->owner == world->GetPlayer() &&
                payload_ptr->id != ItemID::None) {
              const ItemDatabase &db = ItemDatabase::instance();

              if (db.IsOfCategory(payload_ptr->id, ItemCategory::Buildable)) {
                isPreviewingBuilding = true;
                previewingItemID = payload_ptr->id;
                CreatePreviewEntity(payload_ptr->id);
              }
            }
            // End handling player inventory
          }
          // End handling item dragging
        }

        // Update preview entity position while dragging
        if (isPreviewingBuilding && previewEntity != INVALID_ENTITY) {
          UpdatePreviewEntity();
        }
      }

      // Not Dragging with left mouse button
      else {
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
    }

    // Non UI mouse button event
    else if (event.type == SDL_MOUSEBUTTONDOWN) {
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
    } else if (event.type == SDL_MOUSEWHEEL) {
      inputState.mousewheel = {event.wheel.x, event.wheel.y};
    }

    if (event.type == SDL_MOUSEMOTION) {
      inputState.mousedelta = {event.motion.xrel, event.motion.yrel};
      inputState.mousepos = {event.motion.x, event.motion.y};
    }

    // Handle Keyboard Event

    // UI Keyboard Event
    if (io.WantCaptureKeyboard) {
    }
    // Non UI Keyboard Event
    else {
      auto keystate = SDL_GetKeyboardState(nullptr);

      if (keystate[SDL_SCANCODE_K])
        HandleInputAction(InputAction::Debug, InputType::MOUSE);

      HandleInputAxis(keystate);
    }

    if ((event.type == SDL_KEYDOWN && event.key.repeat == 0) ||
        event.type == SDL_KEYUP) {
      auto scancode = event.key.keysym.scancode;

      auto it = keyBindings.find(
          KeyEvent{scancode, static_cast<SDL_EventType>(event.type)});

      // Handle assigned key binding
      if (it != keyBindings.end()) {
        HandleInputAction(it->second, InputType::KEYBOARD);
      }
    }
  }
}

void InputSystem::HandleInputAction(InputAction action, InputType type,
                                    void *params) {
  registry->GetInputState().xAxis = 0;
  registry->GetInputState().yAxis = 0;

  EntityID player = world->GetPlayer();

  switch (action) {
    case InputAction::MouseDrop: {
      ImGuiContext *ctx = static_cast<ImGuiContext *>(params);

      if (ctx->DragDropPayload.DataSize == sizeof(ItemPayload)) {
        ItemPayload *payload_ptr =
            static_cast<ItemPayload *>(ctx->DragDropPayload.Data);

        if (payload_ptr->id != ItemID::None && payload_ptr->amount > 0) {
          const ItemDatabase &db = ItemDatabase::instance();

          Vec2f mouseWorldPos = util::ScreenToWorld(
              registry->GetInputState().mousepos,
              util::GetCameraPosition(registry), screenSize);

          // only handle item inside player's inventory
          if (payload_ptr->owner != player) break;

          // Check if we're dragging to place building
          if (db.IsOfCategory(payload_ptr->id, ItemCategory::Buildable) &&
              isPreviewingBuilding) {
            Vec2 tileIndex =
                world->GetTileIndexFromWorldPosition(mouseWorldPos);

            Vec2f snapWorldPos = (tileIndex * TILE_PIXEL_SIZE);

            EntityID newBuilding = INVALID_ENTITY;
            if (payload_ptr->id == ItemID::AssemblingMachine) {
              if (world->CanPlaceBuilding(tileIndex, 2, 2)) {
                newBuilding =
                    factory->CreateAssemblingMachine(world, snapWorldPos);
              }
            } else if (payload_ptr->id == ItemID::MiningDrill) {
              if (world->CanPlaceBuilding(tileIndex, 1, 1)) {
                newBuilding = factory->CreateMiningDrill(world, snapWorldPos);
              }
            }

            if (newBuilding != INVALID_ENTITY) {
              eventDispatcher->Publish(
                  ItemConsumeEvent{player, payload_ptr->id, 1});
            }

            isPreviewingBuilding = false;
            previewingItemID = ItemID::None;
            DestroyPreviewEntity();
          }

          // Handle non-buildable item drop (create item entity on ground)
          else if (!db.IsOfCategory(payload_ptr->id, ItemCategory::Buildable)) {
            EntityID itemEntity = registry->CreateEntity();

            registry->EmplaceComponent<TransformComponent>(
                itemEntity, TransformComponent{mouseWorldPos});

            const ItemData &itemData = db.get(payload_ptr->id);
            SDL_Texture *texture = assetManager->getTexture(itemData.icon);

            SpriteComponent sprite;
            sprite.texture = texture;
            sprite.srcRect = {ICONSIZE_SMALL_XSTART, 0, ICONSIZE_SMALL,
                              ICONSIZE_SMALL};
            sprite.renderRect = {0, 0, ICONSIZE_SMALL, ICONSIZE_SMALL};
            sprite.renderOrder = 0;
            registry->EmplaceComponent<SpriteComponent>(itemEntity, sprite);

            eventDispatcher->Publish(
                ItemConsumeEvent{player, payload_ptr->id, 1});

            std::cout << "Dropped " << (const char *)itemData.name.c_str()
                      << " at world position " << mouseWorldPos.x << ","
                      << mouseWorldPos.y << std::endl;

            ctx->DragDropPayload.Clear();
          }
        }
      }
    } break;

    // Player started interacting
    case InputAction::StartInteraction: {
      if (!registry->GetComponent<PlayerStateComponent>(player).isMining) {
        const auto &playerTransform =
            registry->GetComponent<TransformComponent>(player);

        std::optional<Vec2f> targetPos;

        // Mouse interaction
        if (type == InputType::MOUSE) {
          const Vec2f campos = util::GetCameraPosition(registry);
          const Vec2f mousepos = registry->GetInputState().mousepos;
          targetPos = util::ScreenToWorld(mousepos, campos, screenSize);
          const double dist =
              util::dist(playerTransform.position, targetPos.value());
          if (maxInteractionRadius < dist) break;
        }

        // Keyboard interaction
        else if (type == InputType::KEYBOARD) {
          targetPos =
              Vec2f(playerTransform.position.x, playerTransform.position.y);
        }

        if (!targetPos.has_value()) break;

        Vec2 tileindex =
            world->GetTileIndexFromWorldPosition(targetPos.value());

        eventDispatcher->Publish(PlayerInteractEvent(tileindex));
      }
      break;
    }

    // Player ended interacting
    case InputAction::StopInteraction: {
      if (registry->HasComponent<PlayerStateComponent>(player)) {
        auto &playerStateComp =
            registry->GetComponent<PlayerStateComponent>(player);
        if (playerStateComp.isMining) {
          playerStateComp.isMining = false;
          auto &animComp = registry->GetComponent<AnimationComponent>(player);
          util::SetAnimation(AnimationName::PLAYER_IDLE, animComp, true);
          util::DetachTimer(registry, timerManager, player, TimerId::Mine);
        }
      }
      break;
    }

    // Open Inventory
    case InputAction::Inventory:
      eventDispatcher->Publish(ToggleInventoryEvent{});
      break;

    // Debug
    case InputAction::Debug: {
      auto playerPosition =
          registry->GetComponent<TransformComponent>(player).position;
      std::cout << "player pos:" << playerPosition << std::endl;

      int playerChunkX =
          std::floor(playerPosition.x / (CHUNK_WIDTH * TILE_PIXEL_SIZE));
      int playerChunkY =
          std::floor(playerPosition.y / (CHUNK_HEIGHT * TILE_PIXEL_SIZE));

      Vec2 playerTileIndex =
          world->GetTileIndexFromWorldPosition(playerPosition);
      auto playerTile = world->GetTileAtWorldPosition(playerPosition);
      auto playerTileformIndex = world->GetTileAtTileIndex(playerTileIndex);
      bool a = world->CanPlaceBuilding(playerTileIndex, 1, 1);
      std::optional<ResourceNodeComponent> rsnode;
      std::optional<TransformComponent> trs;
      if (registry->HasComponent<ResourceNodeComponent>(
              playerTile->occupyingEntity))
        rsnode = registry->GetComponent<ResourceNodeComponent>(
            playerTile->occupyingEntity);
      if (registry->HasComponent<TransformComponent>(
              playerTile->occupyingEntity))
        trs = registry->GetComponent<TransformComponent>(
            playerTile->occupyingEntity);
      bool b = registry->HasComponent<InactiveComponent>(
          playerTileformIndex->occupyingEntity);
      std::cout << "";
      break;
    }

    // Quit game
    case InputAction::Quit:
      eventDispatcher->Publish(QuitEvent{});
      break;
  }
}

void InputSystem::HandleInputAxis(const Uint8 *keyState) {
  // Move Up
  if (keyState[SDL_SCANCODE_W]) {
    registry->GetInputState().yAxis = -1.f;
  }
  // Move Down
  else if (keyState[SDL_SCANCODE_S]) {
    registry->GetInputState().yAxis = 1.f;
  }
  // Stop Y Axis
  else {
    registry->GetInputState().yAxis = 0.f;
  }

  // Move left
  if (keyState[SDL_SCANCODE_A]) {
    registry->GetInputState().xAxis = -1.f;
  }
  // Move Right
  else if (keyState[SDL_SCANCODE_D]) {
    registry->GetInputState().xAxis = 1.f;
  }
  // Stop X Axis
  else {
    registry->GetInputState().xAxis = 0.f;
  }
}

void InputSystem::CreatePreviewEntity(ItemID itemID) {
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
  const ItemDatabase &db = ItemDatabase::instance();
  const ItemData &itemData = db.get(itemID);
  SDL_Texture *texture = assetManager->getTexture(itemData.icon);

  if (texture) {
    SpriteComponent sprite;
    sprite.texture = texture;
    sprite.srcRect = {0, 0, ICONSIZE_BIG, ICONSIZE_BIG};
    sprite.renderRect = {0, 0, TILE_PIXEL_SIZE * width,
                         TILE_PIXEL_SIZE * height};
    sprite.renderOrder = 1000;  // Render on top
    registry->EmplaceComponent<SpriteComponent>(previewEntity, sprite);
  }
}

void InputSystem::DestroyPreviewEntity() {
  if (previewEntity != INVALID_ENTITY) {
    registry->DestroyEntity(previewEntity);
    previewEntity = INVALID_ENTITY;
  }
}

void InputSystem::UpdatePreviewEntity() {
  if (previewEntity == INVALID_ENTITY) return;

  auto camera = registry->view<CameraComponent>();
  if (camera.empty()) return;

  Vec2f mouseWorldPos =
      util::ScreenToWorld(registry->GetInputState().mousepos,
                          util::GetCameraPosition(registry), screenSize);

  Vec2 tileIndex = world->GetTileIndexFromWorldPosition(mouseWorldPos);

  auto &previewComp =
      registry->GetComponent<BuildingPreviewComponent>(previewEntity);

  Vec2f snapWorldPos = (tileIndex * TILE_PIXEL_SIZE);

  auto &transform = registry->GetComponent<TransformComponent>(previewEntity);
  transform.position = snapWorldPos;
}

InputSystem::~InputSystem() = default;
