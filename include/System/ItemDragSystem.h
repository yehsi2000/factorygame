#ifndef SYSTEM_ITEMDRAGSYSTEM_
#define SYSTEM_ITEMDRAGSYSTEM_

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

  bool bIsPreviewingBuilding;
  bool bIsBuildingPlaced;
  ItemID previewingItemID;
  EntityID previewEntity;
  std::unique_ptr<EventHandle> itemDropHandle;

 public:
  ItemDragSystem(const SystemContext& context);
  ~ItemDragSystem();
  void Update();

 private:
  void DestroyPreviewEntity();
  void CreatePreviewEntity(ItemID itemID);
  void ItemDropEventHandler(const ItemDropInWorldEvent& event);
  void UpdatePreviewEntity();
};

#endif /* SYSTEM_ITEMDRAGSYSTEM_ */
