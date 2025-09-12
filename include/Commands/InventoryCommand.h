#ifndef COMMANDS_INVENTORYCOMMAND_
#define COMMANDS_INVENTORYCOMMAND_

#include <algorithm>

#include "Components/InventoryComponent.h"
#include "Commands/Command.h"
#include "Core/Registry.h"
#include "Core/EventDispatcher.h"
#include "Core/World.h"
#include "Core/Item.h"

class InventoryCommand : public Command {
 public:
  InventoryCommand(EntityID target, ItemID item, int amount,
                   EntityID instigator = INVALID_ENTITY)
      : target(target), item(item), amount(amount), instigator(instigator) {}

  void Execute(Registry *registry, EventDispatcher* eventDispatcher, World* world) override {
    if (!registry || !eventDispatcher || !world) return;
    if (!registry->HasComponent<InventoryComponent>(target)) return;

    InventoryComponent &targetInventory =
        registry->GetComponent<InventoryComponent>(target);

    if (instigator != INVALID_ENTITY) {
      // Item Transfer
      if (!registry->HasComponent<InventoryComponent>(instigator)) return;
      InventoryComponent &instigatorInventory =
          registry->GetComponent<InventoryComponent>(instigator);
      if (amount > 0) {
        int actualCnt = TryConsumeItem(instigatorInventory, amount);
        TryAddItem(targetInventory, actualCnt);
      }
    } else {
      // Simple Item Add, Remove
      if (amount > 0)
        TryAddItem(targetInventory, amount);
      else
        TryConsumeItem(targetInventory, -amount);
    }
    // add item

    // consume item
  }

 private:
  EntityID target;
  ItemID item;
  int amount;
  EntityID instigator;

  int TryAddItem(InventoryComponent &inventory, int amount) {
    int actualAddedAmt = 0;
    if (amount <= 0) return actualAddedAmt;

    int maxStackSize = ItemDatabase::instance().get(item).maxStackSize;
    auto it = std::find_if(inventory.items.begin(), inventory.items.end(),
                           [this, maxStackSize](const auto &itempair) {
                             return (itempair.first == this->item &&
                                     itempair.second < maxStackSize);
                           });
    if (it != inventory.items.end()) {
      it->second += amount;
      if (it->second > maxStackSize) {
        actualAddedAmt = maxStackSize - it->second;
        amount = (it->second - maxStackSize);
        it->second = maxStackSize;
      } else {
        actualAddedAmt += amount;
        amount = 0;
      }
    }
    while (amount > maxStackSize) {
      amount -= maxStackSize;
      actualAddedAmt += maxStackSize;
      inventory.items.push_back(std::make_pair(item, maxStackSize));
    }
    if (amount > 0) {
      actualAddedAmt += amount;
      inventory.items.push_back(std::make_pair(item, amount));
    }

    return actualAddedAmt;
  }

  int TryConsumeItem(InventoryComponent &inventory, int amount) {
    int actualConsumeAmt = 0;
    if (amount <= 0) return actualConsumeAmt;

    
    while (amount) {
      auto it = std::find_if(inventory.items.begin(), inventory.items.end(),
                             [this](const auto &itempair) {
                               return itempair.first == this->item;
                             });
      if (it == inventory.items.end()) return 0;
      if (it->second > amount) {
        it->second -= amount;
        actualConsumeAmt += amount;
        break;
      }
      amount -= it->second;
      actualConsumeAmt += it->second;
      inventory.items.erase(it);
    }
    return actualConsumeAmt;
  }
};

#endif/* COMMANDS_INVENTORYCOMMAND_ */
