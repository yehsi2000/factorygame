#include "System/InputSystem.h"

#include <Components/CameraComponent.h>
#include <Components/PlayerStateComponent.h>

#include <optional>

#include "Common.h"
#include "Components/BuildingPreviewComponent.h"
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
#include "boost/functional/hash.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_internal.h"
#include <Components/InactiveComponent.h>

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
      if(keystate[SDL_SCANCODE_K])
        HandleInputAction(InputAction::Debug, InputType::MOUSE);
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
  Registry* reg = engine->GetRegistry();
  reg->GetInputState().xAxis = 0;
  reg->GetInputState().yAxis = 0;

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

          Vec2f mouseWorldPos = util::ScreenToWorld(
              reg->GetInputState().mousepos,
              util::GetCameraPosition(reg), engine->GetScreenSize());

          if (db.IsOfCategory(payload_ptr->id, ItemCategory::Buildable)) {
            Vec2 tileIndex = engine->GetWorld()->GetTileIndexFromWorldPosition(
                mouseWorldPos);

            Vec2f snapWorldPos = (tileIndex * TILE_PIXEL_SIZE);

            EntityID newBuilding = INVALID_ENTITY;
            if (payload_ptr->id == ItemID::AssemblingMachine) {
              if (engine->GetWorld()->CanPlaceBuilding(tileIndex, 2, 2)) {
                newBuilding = factory::CreateAssemblingMachine(
                    reg, engine->GetWorld(), engine->GetRenderer(),
                    snapWorldPos);
              }
            } else if (payload_ptr->id == ItemID::MiningDrill) {
              if (engine->GetWorld()->CanPlaceBuilding(tileIndex, 1, 1)) {
                newBuilding = factory::CreateMiningDrill(
                    reg, engine->GetWorld(), engine->GetRenderer(),
                    snapWorldPos);
              }
            }

            if (newBuilding != INVALID_ENTITY) {
              // Successfully placed building, consume the item
              engine->GetDispatcher()->Publish(
                  ItemConsumeEvent{player, payload_ptr->id, 1});

              std::cout << "Successfully placed building at " << tileIndex.x
                        << "," << tileIndex.y << std::endl;
            } else {
              std::cout << "Cannot place building at " << tileIndex.x << ","
                        << tileIndex.y << std::endl;
            }
          } else {
            // Handle non-buildable item dropping (create item entity on ground)
            EntityID itemEntity = reg->CreateEntity();

            reg->EmplaceComponent<TransformComponent>(
                itemEntity, TransformComponent{mouseWorldPos});

            const ItemData& itemData = db.get(payload_ptr->id);
            SDL_Texture* texture = AssetManager::Instance().getTexture(
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

            std::cout << "Dropped " << itemData.name << " at world position "
                      << mouseWorldPos.x << "," << mouseWorldPos.y << std::endl;

            ctx->DragDropPayload.Clear();
          }

          isPreviewingBuilding = false;
          previewingItemID = ItemID::None;
          DestroyPreviewEntity();
        }
      }
    } 
    break;

    case InputAction::StartInteraction: {
      if (!reg->GetComponent<PlayerStateComponent>(player).isMining) {
        const auto& playerTransform =
            reg->GetComponent<TransformComponent>(player);

        std::optional<Vec2f> targetPos;

        // Mouse interaction
        if (type == InputType::MOUSE) {
          const Vec2f campos = util::GetCameraPosition(reg);
          const Vec2f mousepos =
              engine->GetRegistry()->GetInputState().mousepos;
          targetPos =
              util::ScreenToWorld(mousepos, campos, engine->GetScreenSize());
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
            engine->GetWorld()->GetTileIndexFromWorldPosition(targetPos.value());
        std::cout << engine->GetWorld()->GetTileIndexFromWorldPosition(
                         targetPos.value())
                  << std::endl;

        engine->GetDispatcher()->Publish(
            PlayerInteractEvent(tileindex));
      }
    } break;
    case InputAction::StopInteraction: {
      if (reg->HasComponent<PlayerStateComponent>(player)) {
        auto& state = reg->GetComponent<PlayerStateComponent>(player);
        if (state.isMining) {
          state.isMining = false;
          util::DetachTimer(reg, timerManager, player, TimerId::Mine);
        }
      }
    } break;
    case InputAction::Inventory:
      engine->GetDispatcher()->Publish(ToggleInventoryEvent{});
      break;
    case InputAction::Debug: {
      auto playerPosition =
          reg->GetComponent<TransformComponent>(player).position;
      std::cout << "player pos:" << playerPosition << std::endl;

      int playerChunkX =
          std::floor(playerPosition.x / (CHUNK_WIDTH * TILE_PIXEL_SIZE));
      int playerChunkY =
          std::floor(playerPosition.y / (CHUNK_HEIGHT * TILE_PIXEL_SIZE));

      Vec2 playerTileIndex = engine->GetWorld()->GetTileIndexFromWorldPosition(playerPosition);
      auto playerTile = engine->GetWorld()->GetTileAtWorldPosition(playerPosition);
      auto playerTileformIndex = engine->GetWorld()->GetTileAtTileIndex(playerTileIndex);
      bool a = engine->GetWorld()->CanPlaceBuilding(playerTileIndex, 1, 1);
      std::optional<ResourceNodeComponent> rsnode;
      std::optional<TransformComponent> trs;
      if(reg->HasComponent<ResourceNodeComponent>(playerTile->occupyingEntity))
        rsnode = reg->GetComponent<ResourceNodeComponent>(playerTile->occupyingEntity);
      if(reg->HasComponent<TransformComponent>(playerTile->occupyingEntity))
        trs = reg->GetComponent<TransformComponent>(playerTile->occupyingEntity);
      bool b = reg->HasComponent<InactiveComponent>(playerTileformIndex->occupyingEntity);
      std::cout<<"";
    } break;
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
  Registry* reg = engine->GetRegistry();

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
  reg->EmplaceComponent<TransformComponent>(
      previewEntity, TransformComponent{Vec2f(0, 0)});

  // Add preview component
  reg->EmplaceComponent<BuildingPreviewComponent>(
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
    sprite.renderRect = {0, 0, TILE_PIXEL_SIZE * width,
                         TILE_PIXEL_SIZE * height};
    sprite.renderOrder = 1000;  // Render on top
    reg->EmplaceComponent<SpriteComponent>(previewEntity, sprite);
  }
}

void InputSystem::DestroyPreviewEntity() {
  if (previewEntity != INVALID_ENTITY) {
    Registry* reg = engine->GetRegistry();
    reg->DestroyEntity(previewEntity);
    previewEntity = INVALID_ENTITY;
  }
}

void InputSystem::UpdatePreviewEntity() {
  if (previewEntity == INVALID_ENTITY) return;

  Registry* reg = engine->GetRegistry();

  // Get mouse position in world coordinates
  auto camera = reg->view<CameraComponent>();
  if (camera.empty()) return;

  Vec2f mouseWorldPos = util::ScreenToWorld(
      engine->GetRegistry()->GetInputState().mousepos,
      util::GetCameraPosition(reg), engine->GetScreenSize());

  Vec2 tileIndex =
      engine->GetWorld()->GetTileIndexFromWorldPosition(mouseWorldPos);

  // Get preview component to check building size
  auto& previewComp =
      reg->GetComponent<BuildingPreviewComponent>(previewEntity);

  // Update position to snap to tile grid
  Vec2f snapWorldPos = (tileIndex * TILE_PIXEL_SIZE);

  // std::cout << "snap position : " << snapWorldPos.x << "," << snapWorldPos.y
  //           << std::endl;

  // Update transform component
  auto& transform = reg->GetComponent<TransformComponent>(previewEntity);
  transform.position = snapWorldPos;
}
