#include <memory>

class ItemDatabase;
struct InventoryComponent;
enum class ItemID;

class InventorySystem {
 public:
  InventorySystem(std::shared_ptr<ItemDatabase> db) : itemDB(db) {}
  ~InventorySystem();
  bool consume(InventoryComponent& inv, ItemID itemID, int n);
  void add(InventoryComponent& inv, ItemID itemID, int n);
  int get(InventoryComponent& inv, ItemID itemID) const;

 private:
  std::shared_ptr<ItemDatabase> itemDB;
};