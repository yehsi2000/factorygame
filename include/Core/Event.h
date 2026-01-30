#pragma once
 
#include <memory>
#include <string>
#include <utility>

#include "Core/Entity.h"
#include "Core/Item.h"
#include "Core/Packet.h"
#include "DataStruct/Type.h"

/**
 * @brief The base struct for all events in the game.
 * @details Events are used for immediate, synchronous communication between
 * different systems. When an event is published, it is dispatched to all
 * subscribed listeners in the same frame. This is suitable for decoupled
 * communication where an immediate response is required.
 */
struct Event {
  virtual ~Event() = default;
};

struct EntityDestroyedEvent : public Event {
  explicit EntityDestroyedEvent(Entity entity) : entity(entity) {}
  Entity entity;
};

struct PlayerInteractEvent : public Event {
  explicit PlayerInteractEvent(Vec2f t) : target(t) {}
  Vec2f target;
};

struct PlayerEndInteractEvent : public Event {
  PlayerEndInteractEvent() {}
};

struct ItemAddEvent : public Event {
  ItemAddEvent(Entity target, ItemID item, int amt)
      : target(target), item(item), amount(amt) {}
  Entity target;
  ItemID item;
  int amount;
};

struct ItemConsumeEvent : public Event {
  ItemConsumeEvent(Entity target, ItemID item, int amt)
      : target(target), item(item), amount(amt) {}
  Entity target;
  ItemID item;
  int amount;
};

struct ItemMoveEvent : public Event {
  ItemMoveEvent(Entity source, Entity dest, ItemID item, int amt)
      : source(source), dest(dest), item(item), amount(amt) {}
  Entity source;
  Entity dest;
  ItemID item;
  int amount;
};

struct ItemDropInWorldEvent : public Event {
  ItemDropInWorldEvent(const Vec2f worldPos, ItemPayload payload)
      : worldPos(worldPos), payload(payload) {}
  ItemPayload payload;
  const Vec2f worldPos;
};

struct AssemblyAddInputEvent : public Event {
  AssemblyAddInputEvent(Entity machine, Entity target, ItemID item, int amt)
      : machine(machine), target(target), item(item), amount(amt) {}
  Entity machine;
  Entity target;
  ItemID item;
  int amount;
};

struct AssemblyTakeOutputEvent : public Event {
  AssemblyTakeOutputEvent(Entity machine, Entity target, ItemID item,
                          int amt)
      : machine(machine), target(target), item(item), amount(amt) {}
  Entity machine;
  Entity target;
  ItemID item;
  int amount;
};

struct AssemblyCraftOutputEvent : public Event {
  explicit AssemblyCraftOutputEvent(Entity machine) : machine(machine) {}
  Entity machine;
};

struct ToggleInventoryEvent : public Event {
  ToggleInventoryEvent() = default;
};

struct ToggleChatInputEvent : public Event {
  ToggleChatInputEvent() = default;
};

struct SendChatEvent : public Event {
  explicit SendChatEvent(std::shared_ptr<std::string> msg) : message(std::move(msg)) {}
  std::shared_ptr<std::string> message;
};

struct NewChatEvent : public Event {
  NewChatEvent(clientid_t id, std::shared_ptr<std::string> msg)
      : id(id), message(std::move(msg)) {}
  clientid_t id;
  std::shared_ptr<std::string> message;
};

struct QuitEvent : public Event {};
