#ifndef COMMANDS_INVENTORYCOMMAND_
#define COMMANDS_INVENTORYCOMMAND_

#include <algorithm>

#include "Core/Command.h"
#include "Core/Event.h"
#include "Core/GEngine.h"
#include "Core/Item.h"

// A command to execute mining interaction with resource node
class InventoryCommand : public Command {
 public:
  InventoryCommand(EntityID target, ItemID item, int amount)
      : target(target), item(item), amount(amount) {}

  void Execute(GEngine &engine, Registry &registry) override {
    if (!registry.HasComponent<InventoryComponent>(target)) return;

    InventoryComponent &inventory =
        registry.GetComponent<InventoryComponent>(target);
    // add item
    if (amount > 0) {
      auto it = std::find_if(inventory.items.begin(), inventory.items.end(),
                             [this](const auto &itempair) {
                               return itempair.first == this->item;
                             });
      int maxStackSize = ItemDatabase::instance().get(item).maxStackSize;
      if (it != inventory.items.end()) {
        it->second += amount;
        if (it->second > maxStackSize) {
          amount -= (maxStackSize - it->second);
          it->second = maxStackSize;
        } else {
          amount = 0;
        }
            }
      while (amount > maxStackSize) {
        amount -= maxStackSize;
        inventory.items.push_back(std::make_pair(item, maxStackSize));
      }
      if (amount > 0) {
        inventory.items.push_back(std::make_pair(item, amount));
      }
    }
    // consume item
    else {
      while (amount) {
        auto it = std::find_if(inventory.items.begin(), inventory.items.end(),
                               [this](const auto &itempair) {
                                 return itempair.first == this->item;
                               });
        if (it == inventory.items.end()) return;
        if (it->second > amount) {
          it->second -= amount;
          break;
        }
        amount -= it->second;
        inventory.items.erase(it);
      }
    }
  }

 private:
  EntityID target;
  ItemID item;
  int amount;
};

#endif /* COMMANDS_INVENTORYCOMMAND_ */
