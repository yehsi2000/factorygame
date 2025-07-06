#include <memory>

#include "Entity.h"

class ItemDatabase;
struct ResourceNodeComponent;
class Registry;

class ResourceNodeSystem {
 public:
  ResourceNodeSystem(std::shared_ptr<ItemDatabase> db, Registry* r);
  void Update();
  void AddMiner(ResourceNodeComponent& resNode, EntityID player);
  void RemoveMiner(ResourceNodeComponent& resNode);
  long long leftcount(ResourceNodeComponent& resNode);
  ~ResourceNodeSystem();

 private:
  std::shared_ptr<ItemDatabase> itemDatabase;
  Registry* registry;
};