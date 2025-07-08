#include "System/InventorySystem.h"

#include "Components/InventoryComponent.h"

InventorySystem::~InventorySystem() = default;

bool InventorySystem::consume(InventoryComponent& inv, ItemID itemID, int n) {
  if (inv.items.find(itemID) != inv.items.end() && inv.items[itemID] >= n) {
    inv.items[itemID] -= n;
    if (inv.items[itemID] == 0) inv.items.erase(itemID);
    return true;
  }
  return false;
}

void InventorySystem::add(InventoryComponent& inv, ItemID itemID, int n) {
  inv.items[itemID] += n;
}

int InventorySystem::get(InventoryComponent& inv, ItemID itemID) const {
  auto pos = inv.items.find(itemID);
  return (pos != inv.items.end()) ? pos->second : 0;
}
