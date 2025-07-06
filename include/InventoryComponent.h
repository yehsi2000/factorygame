#include <unordered_map>

enum class ItemID;

struct InventoryComponent {
  std::unordered_map<ItemID, int> items;
};
