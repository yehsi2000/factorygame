#ifndef CORE_EVENT_
#define CORE_EVENT_

#include <string>
#include <utility>
#include <memory>

#include "Core/Entity.h"
#include "Core/Item.h"
#include "Core/Type.h"


struct Event {
  virtual ~Event() = default;
};

struct PlayerInteractEvent : public Event {
  PlayerInteractEvent(Vec2f t) : target(t) {}
  Vec2f target;
};

struct PlayerEndInteractEvent : public Event {
  PlayerEndInteractEvent() {}
};

struct ItemAddEvent : public Event {
  ItemAddEvent(EntityID target, ItemID item, int amt)
      : target(target), item(item), amount(amt) {}
  EntityID target;
  ItemID item;
  int amount;
};

struct ItemConsumeEvent : public Event {
  ItemConsumeEvent(EntityID target, ItemID item, int amt)
      : target(target), item(item), amount(amt) {}
  EntityID target;
  ItemID item;
  int amount;
};

struct ItemMoveEvent : public Event {
  ItemMoveEvent(EntityID source, EntityID dest, ItemID item, int amt)
      : source(source), dest(dest), item(item), amount(amt) {}
  EntityID source;
  EntityID dest;
  ItemID item;
  int amount;
};

struct ItemDropInWorldEvent : public Event {
  ItemDropInWorldEvent(const ItemPayload& payload) : payload(payload) {}
  ItemPayload payload;
};

struct AssemblyAddInputEvent : public Event {
  AssemblyAddInputEvent(EntityID machine, EntityID target, ItemID item, int amt)
      : machine(machine), target(target), item(item), amount(amt) {}
  EntityID machine;
  EntityID target;
  ItemID item;
  int amount;
};

struct AssemblyTakeOutputEvent : public Event {
  AssemblyTakeOutputEvent(EntityID machine, EntityID target, ItemID item,
                          int amt)
      : machine(machine), target(target), item(item), amount(amt) {}
  EntityID machine;
  EntityID target;
  ItemID item;
  int amount;
};

struct AssemblyCraftOutputEvent : public Event {
  AssemblyCraftOutputEvent(EntityID machine) : machine(machine) {}
  EntityID machine;
};

struct ToggleInventoryEvent : public Event {
  ToggleInventoryEvent() = default;
};

struct ToggleChatInputEvent : public Event {
  ToggleChatInputEvent() = default;
};

struct SendChatEvent : public Event {
  SendChatEvent(std::shared_ptr<std::string> msg) : message(msg) {}
  std::shared_ptr<std::string> message;
};

struct NewChatEvent : public Event {
  NewChatEvent(std::shared_ptr<std::string> msg) : message(msg) {}
  std::shared_ptr<std::string> message;
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

#endif/* CORE_EVENT_ */
