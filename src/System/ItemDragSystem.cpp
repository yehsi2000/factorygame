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
#include "Core/InputPoller.h"
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
      inputPoller(context.inputPoller),
      eventDispatcher(context.eventDispatcher),
      factory(context.entityFactory),
      isPreviewingBuilding(false),
      isBuildingPlaced(false),
      previewEntity(INVALID_ENTITY),
      previewingItemID(ItemID::None) {
  itemDropHandle = eventDispatcher->Subscribe<MouseDropEvent>(
      [this](const MouseDropEvent &event) {
        this->ItemDropEventHandler(event);
      });
}

void ItemDragSystem::Update() {
  if (!inputPoller->IsDraggingOutSide()) {
    return;
  }
  isBuildingPlaced = false;

  ImGuiContext *ctx = ImGui::GetCurrentContext();

  // Handle item dragging
  if (!isPreviewingBuilding &&
      ctx->DragDropPayload.DataSize == sizeof(ItemPayload)) {
    ItemPayload *payload_ptr =
        static_cast<ItemPayload *>(ctx->DragDropPayload.Data);

    // Only handle dragging from player inventory
    if (payload_ptr->owner == world->GetPlayer() &&
        payload_ptr->id != ItemID::None) {
      const ItemDatabase &db = ItemDatabase::instance();

      if (db.IsOfCategory(payload_ptr->id, ItemCategory::Buildable)) {
        previewingItemID = payload_ptr->id;
        CreatePreviewEntity(payload_ptr->id);
      }
    }
  }
  UpdatePreviewEntity();
}

void ItemDragSystem::UpdatePreviewEntity() {
  if (!isPreviewingBuilding) return;

  Vec2f mouseWorldPos = util::ScreenToWorld(inputPoller->GetMousePositon(),
                                            util::GetCameraPosition(registry),
                                            inputPoller->GetScreenSize());

  Vec2 tileIndex = world->GetTileIndexFromWorldPosition(mouseWorldPos);

  auto &previewComp =
      registry->GetComponent<BuildingPreviewComponent>(previewEntity);

  Vec2f snapWorldPos = (tileIndex * TILE_PIXEL_SIZE);

  auto &transform = registry->GetComponent<TransformComponent>(previewEntity);
  transform.position = snapWorldPos;
}

void ItemDragSystem::CreatePreviewEntity(ItemID itemID) {
  // Destroy existing preview entity if it exists
  if (previewEntity != INVALID_ENTITY || isBuildingPlaced)
    return;  // DestroyPreviewEntity();

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

void ItemDragSystem::ItemDropEventHandler(const MouseDropEvent &event) {
  ImGuiContext *ctx = ImGui::GetCurrentContext();
  EntityID player = world->GetPlayer();

  if (ctx->DragDropPayload.DataSize == sizeof(ItemPayload)) {
    ItemPayload *payload_ptr =
        static_cast<ItemPayload *>(ctx->DragDropPayload.Data);

    if (payload_ptr->id != ItemID::None && payload_ptr->amount > 0) {
      const ItemDatabase &db = ItemDatabase::instance();

      Vec2f mouseWorldPos = util::ScreenToWorld(
          inputPoller->GetMousePositon(), util::GetCameraPosition(registry),
          inputPoller->GetScreenSize());

      // only handle item inside player's inventory
      if (payload_ptr->owner != player) return;

      // Check if we're dragging to place building
      if (db.IsOfCategory(payload_ptr->id, ItemCategory::Buildable)) {
        Vec2 tileIndex = world->GetTileIndexFromWorldPosition(mouseWorldPos);

        Vec2f snapWorldPos = (tileIndex * TILE_PIXEL_SIZE);

        EntityID newBuilding = INVALID_ENTITY;
        if (payload_ptr->id == ItemID::AssemblingMachine) {
          if (world->HasNoOcuupyingEntity(tileIndex, 2, 2)) {
            newBuilding = factory->CreateAssemblingMachine(world, snapWorldPos);
          }
        } else if (payload_ptr->id == ItemID::MiningDrill) {
          if (world->HasNoOcuupyingEntity(tileIndex, 1, 1)) {
            newBuilding = factory->CreateMiningDrill(world, snapWorldPos);
          }
        }

        if (newBuilding != INVALID_ENTITY) {
          eventDispatcher->Publish(
              ItemConsumeEvent{player, payload_ptr->id, 1});
        }

        DestroyPreviewEntity();
        ctx->DragDropPayload.Clear();
        isBuildingPlaced = true;
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

        eventDispatcher->Publish(ItemConsumeEvent{player, payload_ptr->id, 1});

        std::cout << "Dropped " << (const char *)itemData.name.c_str()
                  << " at world position " << mouseWorldPos.x << ","
                  << mouseWorldPos.y << std::endl;

        ctx->DragDropPayload.Clear();
      }
    }
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