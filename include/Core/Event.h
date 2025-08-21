#ifndef CORE_EVENT_
#define CORE_EVENT_

#include "Core/Entity.h"
#include "Core/Item.h"

struct Event {
  virtual ~Event() = default;
};

struct PlayerInteractEvent : public Event {
  PlayerInteractEvent(EntityID i) : target(i) {}
  EntityID target;
};

struct ItemAddEvent : public Event {
  ItemAddEvent(EntityID target, ItemID item, int amt)
      : target(target), item(item), amount(amt) {};
  EntityID target;
  ItemID item;
  int amount;
};

struct ItemConsumeEvent : public Event {
  ItemConsumeEvent(EntityID target, ItemID item, int amt)
      : target(target), item(item), amount(amt) {};
  EntityID target;
  ItemID item;
  int amount;
};

struct ToggleInventoryEvent : public Event {
  ToggleInventoryEvent() = default;
};

struct XAxisEvent : public Event {
  XAxisEvent(float i) : val(i) {}
  float val;
};

struct YAxisEvent : public Event {
  YAxisEvent(float i) : val(i) {}
  float val;
};

struct QuitEvent : public Event {};

#endif /* CORE_EVENT_ */
