#pragma once
#include <memory>
#include "Entity.h"
#include "Inventory.h"
#include "Event.h"

class Player : public Entity {
  std::unique_ptr<Inventory> inv;
  CallbackID OreEventId;
 public:
  Player();
  virtual ~Player();
  Inventory* GetInventory() const;
};
