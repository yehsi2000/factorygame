#pragma once

#include <memory>

#include "Core/Entity.h"
#include "Core/Event.h"
#include "Core/Item.h"
#include "Core/SystemContext.h"

class EventHandle;

class ItemDragSystem {
  Registry* registry;
  World* world;
  AssetManager* assetManager;
  InputManager* inputManager;
  EventDispatcher* eventDispatcher;
  EntityFactory* factory;

  bool isPreviewingBuilding;
  bool isBuildingPlaced;
  ItemID previewingItemID;
  Entity previewEntity;
  std::unique_ptr<EventHandle> itemDropHandle;

 public:
  explicit ItemDragSystem(const SystemContext& context);
  ~ItemDragSystem();
  void Update();

 private:
  void DestroyPreviewEntity();
  void CreatePreviewEntity(ItemID itemID);
  void ItemDropEventHandler(const ItemDropInWorldEvent& event);
  void UpdatePreviewEntity();
};
