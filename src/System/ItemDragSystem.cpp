#include "System/ItemDragSystem.h"

#include <memory>

#include "Common.h"
#include "Components/BuildingPreviewComponent.h"
#include "Components/CameraComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TransformComponent.h"
#include "Core/AssetManager.h"
#include "Core/Entity.h"
#include "Core/EntityFactory.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/InputManager.h"
#include "Core/Item.h"
#include "Core/Registry.h"
#include "Core/World.h"
#include "Util/CameraUtil.h"
#include "imgui.h"
#include "imgui_internal.h"

ItemDragSystem::ItemDragSystem(const SystemContext &context)
    : registry(context.registry),
      world(context.world),
      assetManager(context.assetManager),
      inputManager(context.inputManager),
      eventDispatcher(context.eventDispatcher),
      factory(context.entityFactory),
      isPreviewingBuilding(false),
      isBuildingPlaced(false),
      previewEntity(INVALID_ENTITY),
      previewingItemID(ItemID::None) {
  itemDropHandle = eventDispatcher->Subscribe<ItemDropInWorldEvent>(
      [this](const ItemDropInWorldEvent &event) {
        this->ItemDropEventHandler(event);
      });
}

void ItemDragSystem::Update() {
  isBuildingPlaced = false;

  if (!ImGui::GetIO().WantCaptureMouse) return;

  const ImGuiPayload *payload_ptr = ImGui::GetDragDropPayload();

  // Handle item dragging
  if (!isPreviewingBuilding && payload_ptr != nullptr &&
      payload_ptr->DataSize == sizeof(ItemPayload)) {
    ItemPayload *itemPayload = static_cast<ItemPayload *>(payload_ptr->Data);

    // Only handle dragging from player inventory
    if (itemPayload->owner == world->GetPlayer() &&
        itemPayload->id != ItemID::None) {
      const ItemDatabase &db = ItemDatabase::instance();

      if (db.IsOfCategory(itemPayload->id, ItemCategory::Buildable)) {
        previewingItemID = itemPayload->id;
        CreatePreviewEntity(itemPayload->id);
      }
    }
  } else if (isPreviewingBuilding &&
             (payload_ptr == nullptr ||
              payload_ptr->DataSize != sizeof(ItemPayload))) {
    DestroyPreviewEntity();
  }
  UpdatePreviewEntity();
}

void ItemDragSystem::UpdatePreviewEntity() {
  if (!isPreviewingBuilding) {
    return;
  }

  Vec2f mouseWorldPos = util::ScreenToWorld(inputManager->GetMousePosition(),
                                            util::GetCameraPosition(registry),
                                            inputManager->GetScreenSize());

  Vec2 tileIndex = world->GetTileIndexFromWorldPosition(mouseWorldPos);

  auto &previewComp =
      registry->GetComponent<BuildingPreviewComponent>(previewEntity);

  Vec2f snapWorldPos = (tileIndex * TILE_PIXEL_SIZE);

  auto &transform = registry->GetComponent<TransformComponent>(previewEntity);
  transform.position = snapWorldPos;
}

void ItemDragSystem::CreatePreviewEntity(ItemID itemID) {
  // Destroy existing preview entity if it exists
  if (previewEntity != INVALID_ENTITY || isBuildingPlaced) {
    DestroyPreviewEntity();
    return;
  }

  isPreviewingBuilding = true;
  previewEntity = registry->CreateEntity();

  // Determine building size
  // TODO : move this into itemDB
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

void ItemDragSystem::ItemDropEventHandler(const ItemDropInWorldEvent &event) {
  EntityID player = world->GetPlayer();

  const ItemDatabase &db = ItemDatabase::instance();

  Vec2f mouseWorldPos = util::ScreenToWorld(inputManager->GetMousePosition(),
                                            util::GetCameraPosition(registry),
                                            inputManager->GetScreenSize());

  // only handle item inside player's inventory
  if (event.payload.owner != player) return;

  // Check if we're dragging to place building
  if (db.IsOfCategory(event.payload.id, ItemCategory::Buildable)) {
    Vec2 tileIndex = world->GetTileIndexFromWorldPosition(mouseWorldPos);

    Vec2f snapWorldPos = (tileIndex * TILE_PIXEL_SIZE);

    EntityID newBuilding = INVALID_ENTITY;
    if (event.payload.id == ItemID::AssemblingMachine) {
      if (world->HasNoOcuupyingEntity(tileIndex, 2, 2)) {
        newBuilding = factory->CreateAssemblingMachine(world, snapWorldPos);
      }
    } else if (event.payload.id == ItemID::MiningDrill) {
      if (world->HasNoOcuupyingEntity(tileIndex, 1, 1)) {
        newBuilding = factory->CreateMiningDrill(world, snapWorldPos);
      }
    }

    if (newBuilding != INVALID_ENTITY) {
      eventDispatcher->Publish(ItemConsumeEvent{player, event.payload.id, 1});
    }

    DestroyPreviewEntity();
    isBuildingPlaced = true;
  }

  // Handle non-buildable item drop (create item entity on ground)
  else if (!db.IsOfCategory(event.payload.id, ItemCategory::Buildable)) {
    EntityID itemEntity = registry->CreateEntity();

    registry->EmplaceComponent<TransformComponent>(
        itemEntity, TransformComponent{mouseWorldPos});

    const ItemData &itemData = db.get(event.payload.id);
    SDL_Texture *texture = assetManager->getTexture(itemData.icon);

    SpriteComponent sprite;
    sprite.texture = texture;
    sprite.srcRect = {ICONSIZE_SMALL_XSTART, 0, ICONSIZE_SMALL, ICONSIZE_SMALL};
    sprite.renderRect = {0, 0, ICONSIZE_SMALL, ICONSIZE_SMALL};
    sprite.renderOrder = 0;
    registry->EmplaceComponent<SpriteComponent>(itemEntity, sprite);

    eventDispatcher->Publish(ItemConsumeEvent{player, event.payload.id, 1});

    std::cout << "Dropped " << (const char *)itemData.name.c_str()
              << " at world position " << mouseWorldPos.x << ","
              << mouseWorldPos.y << std::endl;
  }
}

void ItemDragSystem::DestroyPreviewEntity() {
  if (previewEntity != INVALID_ENTITY) {
    isPreviewingBuilding = false;
    previewingItemID = ItemID::None;
    registry->DestroyEntity(previewEntity);
    previewEntity = INVALID_ENTITY;
  }
}

ItemDragSystem::~ItemDragSystem() = default;