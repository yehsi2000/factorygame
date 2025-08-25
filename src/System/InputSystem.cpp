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
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/GEngine.h"
#include "Core/InputState.h"
#include "Core/Item.h"
#include "Core/Registry.h"
#include "Core/TileData.h"
#include "Core/World.h"


#include "Util/AnimUtil.h"
#include "Util/CameraUtil.h"
#include "Util/EntityFactory.h"
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

InputSystem::InputSystem(GEngine *e)
    : engine(e), io(ImGui::GetIO()), isDraggingOutside(false),
      isPreviewingBuilding(false), previewingItemID(ItemID::None),
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
  auto &inputState = engine->GetRegistry()->GetInputState();
  inputState.rightMousePressed = false;
  inputState.rightMouseReleased = false;
  inputState.mouseDeltaX = 0;
  inputState.mouseDeltaY = 0;
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);

    if (event.type == SDL_QUIT) {
      engine->GetDispatcher()->Publish(QuitEvent{});
      return; // Stop all input handling if quit event occurs
    }

    // Handle Mouse Event
    if (io.WantCaptureMouse) {
      // Dragging from UI
      ImGuiContext *ctx = ImGui::GetCurrentContext();
      ImGuiWindow *window = ctx->HoveredWindow;

      if (window == nullptr && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        if (!isDraggingOutside) {
          isDraggingOutside = true;
          std::cout << "start dragging outside!" << std::endl;

          // Handle item dragging
          if (ctx->DragDropPayload.DataSize == sizeof(ItemPayload)) {
            ItemPayload *payload_ptr =
                static_cast<ItemPayload *>(ctx->DragDropPayload.Data);

            // Only handle dragging from player inventory
            if (payload_ptr->owner == engine->GetPlayer() &&
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
  Registry *reg = engine->GetRegistry();
  reg->GetInputState().xAxis = 0;
  reg->GetInputState().yAxis = 0;

  EntityID player = engine->GetPlayer();

  switch (action) {
  case InputAction::MouseDrop: {
    ImGuiContext *ctx = static_cast<ImGuiContext *>(params);

    if (ctx->DragDropPayload.DataSize == sizeof(ItemPayload)) {
      ItemPayload *payload_ptr =
          static_cast<ItemPayload *>(ctx->DragDropPayload.Data);

      if (payload_ptr->id != ItemID::None && payload_ptr->amount > 0) {
        const ItemDatabase &db = ItemDatabase::instance();

        Vec2f mouseWorldPos = util::ScreenToWorld(reg->GetInputState().mousepos,
                                                  util::GetCameraPosition(reg),
                                                  engine->GetScreenSize());

        // only handle item inside player's inventory
        if (payload_ptr->owner != player)
          break;

        // Check if we're dragging to place building
        if (db.IsOfCategory(payload_ptr->id, ItemCategory::Buildable) &&
            isPreviewingBuilding) {
          Vec2 tileIndex =
              engine->GetWorld()->GetTileIndexFromWorldPosition(mouseWorldPos);

          Vec2f snapWorldPos = (tileIndex * TILE_PIXEL_SIZE);

          EntityID newBuilding = INVALID_ENTITY;
          if (payload_ptr->id == ItemID::AssemblingMachine) {
            if (engine->GetWorld()->CanPlaceBuilding(tileIndex, 2, 2)) {
              newBuilding = factory::CreateAssemblingMachine(
                  reg, engine->GetWorld(), engine->GetRenderer(), snapWorldPos);
            }
          } else if (payload_ptr->id == ItemID::MiningDrill) {
            if (engine->GetWorld()->CanPlaceBuilding(tileIndex, 1, 1)) {
              newBuilding = factory::CreateMiningDrill(
                  reg, engine->GetWorld(), engine->GetRenderer(), snapWorldPos);
            }
          }

          if (newBuilding != INVALID_ENTITY) {
            engine->GetDispatcher()->Publish(
                ItemConsumeEvent{player, payload_ptr->id, 1});

            std::cout << "Successfully placed building at " << tileIndex.x
                      << "," << tileIndex.y << std::endl;
          } else {
            std::cout << "Cannot place building at " << tileIndex.x << ","
                      << tileIndex.y << std::endl;
          }

          isPreviewingBuilding = false;
          previewingItemID = ItemID::None;
          DestroyPreviewEntity();
        }

        // Handle non-buildable item drop (create item entity on ground)
        else if (!db.IsOfCategory(payload_ptr->id, ItemCategory::Buildable)) {
          EntityID itemEntity = reg->CreateEntity();

          reg->EmplaceComponent<TransformComponent>(
              itemEntity, TransformComponent{mouseWorldPos});

          const ItemData &itemData = db.get(payload_ptr->id);
          SDL_Texture *texture = AssetManager::Instance().getTexture(
              itemData.icon, engine->GetRenderer());

          SpriteComponent sprite;
          sprite.texture = texture;
          sprite.srcRect = {ICONSIZE_SMALL_XSTART, 0, ICONSIZE_SMALL,
                            ICONSIZE_SMALL};
          sprite.renderRect = {0, 0, ICONSIZE_SMALL, ICONSIZE_SMALL};
          sprite.renderOrder = 0;
          reg->EmplaceComponent<SpriteComponent>(itemEntity, sprite);

          engine->GetDispatcher()->Publish(
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
    if (!reg->GetComponent<PlayerStateComponent>(player).isMining) {
      const auto &playerTransform =
          reg->GetComponent<TransformComponent>(player);

      std::optional<Vec2f> targetPos;

      // Mouse interaction
      if (type == InputType::MOUSE) {
        const Vec2f campos = util::GetCameraPosition(reg);
        const Vec2f mousepos = engine->GetRegistry()->GetInputState().mousepos;
        targetPos =
            util::ScreenToWorld(mousepos, campos, engine->GetScreenSize());
        const double dist =
            util::dist(playerTransform.position, targetPos.value());
        if (maxInteractionRadius < dist)
          break;
      }

      // Keyboard interaction
      else if (type == InputType::KEYBOARD) {
        targetPos =
            Vec2f(playerTransform.position.x, playerTransform.position.y);
      }

      if (!targetPos.has_value())
        break;

      Vec2 tileindex =
          engine->GetWorld()->GetTileIndexFromWorldPosition(targetPos.value());
      std::cout << engine->GetWorld()->GetTileIndexFromWorldPosition(
                       targetPos.value())
                << std::endl;

      engine->GetDispatcher()->Publish(PlayerInteractEvent(tileindex));
    }
    break;
  }

  // Player ended interacting
  case InputAction::StopInteraction: {
    if (reg->HasComponent<PlayerStateComponent>(player)) {
      auto &playerStateComp = reg->GetComponent<PlayerStateComponent>(player);
      if (playerStateComp.isMining) {
        playerStateComp.isMining = false;
        auto &animComp = reg->GetComponent<AnimationComponent>(player);
        util::SetAnimation(AnimationName::PLAYER_IDLE, animComp, true);
        util::DetachTimer(reg, engine->GetTimerManager(), player,
                          TimerId::Mine);
      }
    }
    break;
  }

  // Open Inventory
  case InputAction::Inventory:
    engine->GetDispatcher()->Publish(ToggleInventoryEvent{});
    break;

  // Debug
  case InputAction::Debug: {
    auto playerPosition =
        reg->GetComponent<TransformComponent>(player).position;
    std::cout << "player pos:" << playerPosition << std::endl;

    int playerChunkX =
        std::floor(playerPosition.x / (CHUNK_WIDTH * TILE_PIXEL_SIZE));
    int playerChunkY =
        std::floor(playerPosition.y / (CHUNK_HEIGHT * TILE_PIXEL_SIZE));

    Vec2 playerTileIndex =
        engine->GetWorld()->GetTileIndexFromWorldPosition(playerPosition);
    auto playerTile =
        engine->GetWorld()->GetTileAtWorldPosition(playerPosition);
    auto playerTileformIndex =
        engine->GetWorld()->GetTileAtTileIndex(playerTileIndex);
    bool a = engine->GetWorld()->CanPlaceBuilding(playerTileIndex, 1, 1);
    std::optional<ResourceNodeComponent> rsnode;
    std::optional<TransformComponent> trs;
    if (reg->HasComponent<ResourceNodeComponent>(playerTile->occupyingEntity))
      rsnode =
          reg->GetComponent<ResourceNodeComponent>(playerTile->occupyingEntity);
    if (reg->HasComponent<TransformComponent>(playerTile->occupyingEntity))
      trs = reg->GetComponent<TransformComponent>(playerTile->occupyingEntity);
    bool b = reg->HasComponent<InactiveComponent>(
        playerTileformIndex->occupyingEntity);
    std::cout << "";
    break;
  }

  // Quit game
  case InputAction::Quit:
    engine->GetDispatcher()->Publish(QuitEvent{});
    break;
  }
}

void InputSystem::HandleInputAxis(const Uint8 *keyState) {

  // Move Up
  if (keyState[SDL_SCANCODE_W]) {
    engine->GetRegistry()->GetInputState().yAxis = -1.f;
  }
  // Move Down
  else if (keyState[SDL_SCANCODE_S]) {
    engine->GetRegistry()->GetInputState().yAxis = 1.f;
  }
  // Stop Y Axis
  else {
    engine->GetRegistry()->GetInputState().yAxis = 0.f;
  }

  // Move left
  if (keyState[SDL_SCANCODE_A]) {
    engine->GetRegistry()->GetInputState().xAxis = -1.f;
  }
  // Move Right
  else if (keyState[SDL_SCANCODE_D]) {
    engine->GetRegistry()->GetInputState().xAxis = 1.f;
  }
  // Stop X Axis
  else {
    engine->GetRegistry()->GetInputState().xAxis = 0.f;
  }
}

void InputSystem::CreatePreviewEntity(ItemID itemID) {
  Registry *reg = engine->GetRegistry();

  // Destroy existing preview entity if it exists
  DestroyPreviewEntity();

  previewEntity = reg->CreateEntity();

  // Determine building size
  int width = 1, height = 1;
  if (itemID == ItemID::AssemblingMachine) {
    width = height = 2;
  } else if (itemID == ItemID::MiningDrill) {
    width = height = 1;
  }

  // Add transform component (position will be updated in UpdatePreviewEntity)
  reg->EmplaceComponent<TransformComponent>(previewEntity,
                                            TransformComponent{Vec2f(0, 0)});

  // Add preview component
  reg->EmplaceComponent<BuildingPreviewComponent>(
      previewEntity, BuildingPreviewComponent{itemID, width, height});

  // Add sprite component for preview visual
  const ItemDatabase &db = ItemDatabase::instance();
  const ItemData &itemData = db.get(itemID);
  SDL_Texture *texture =
      AssetManager::Instance().getTexture(itemData.icon, engine->GetRenderer());

  if (texture) {
    SpriteComponent sprite;
    sprite.texture = texture;
    sprite.srcRect = {0, 0, ICONSIZE_BIG, ICONSIZE_BIG};
    sprite.renderRect = {0, 0, TILE_PIXEL_SIZE * width,
                         TILE_PIXEL_SIZE * height};
    sprite.renderOrder = 1000; // Render on top
    reg->EmplaceComponent<SpriteComponent>(previewEntity, sprite);
  }
}

void InputSystem::DestroyPreviewEntity() {
  if (previewEntity != INVALID_ENTITY) {
    Registry *reg = engine->GetRegistry();
    reg->DestroyEntity(previewEntity);
    previewEntity = INVALID_ENTITY;
  }
}

void InputSystem::UpdatePreviewEntity() {
  if (previewEntity == INVALID_ENTITY)
    return;

  Registry *reg = engine->GetRegistry();

  auto camera = reg->view<CameraComponent>();
  if (camera.empty())
    return;

  Vec2f mouseWorldPos = util::ScreenToWorld(
      engine->GetRegistry()->GetInputState().mousepos,
      util::GetCameraPosition(reg), engine->GetScreenSize());

  Vec2 tileIndex =
      engine->GetWorld()->GetTileIndexFromWorldPosition(mouseWorldPos);

  auto &previewComp =
      reg->GetComponent<BuildingPreviewComponent>(previewEntity);

  Vec2f snapWorldPos = (tileIndex * TILE_PIXEL_SIZE);

  auto &transform = reg->GetComponent<TransformComponent>(previewEntity);
  transform.position = snapWorldPos;
}
