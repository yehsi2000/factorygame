#ifndef SYSTEM_INVENTORYSYSTEM_
#define SYSTEM_INVENTORYSYSTEM_

#include <memory>

#include "Core/Event.h"
#include "Core/SystemContext.h"

class EventHandle;

class InventorySystem {
 public:
  InventorySystem(const SystemContext& context);
  ~InventorySystem();

 private:
  Registry* registry;
  EventDispatcher* eventDispatcher;
  CommandQueue* commandQueue;

  std::unique_ptr<EventHandle> addEventHandle;
  std::unique_ptr<EventHandle> consumeEventHandle;
  std::unique_ptr<EventHandle> moveEventHandle;

  void AddItem(const ItemAddEvent& e);
  void ConsumeItem(const ItemConsumeEvent& e);
  void MoveItem(const ItemMoveEvent& e);
};

#endif /* SYSTEM_INVENTORYSYSTEM_ */
