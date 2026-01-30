#pragma once

#include <memory>

#include "Core/Event.h"
#include "Core/SystemContext.h"

class EventHandle;

class InventorySystem {
 public:
  explicit InventorySystem(const SystemContext& context);
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
