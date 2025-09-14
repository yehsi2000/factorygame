#ifndef CORE_EVENT_
#define CORE_EVENT_

#include <memory>
#include <string>
#include <utility>

#include "Core/Entity.h"
#include "Core/Item.h"
#include "Core/Packet.h"
#include "Core/Type.h"

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
  EntityDestroyedEvent(EntityID entity) : entity(entity) {}
  EntityID entity;
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
  ItemDropInWorldEvent(const Vec2f worldPos, ItemPayload payload)
      : worldPos(worldPos), payload(std::move(payload)) {}
  ItemPayload payload;
  const Vec2f worldPos;
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
  NewChatEvent(clientid_t id, std::shared_ptr<std::string> msg)
      : id(id), message(msg) {}
  clientid_t id;
  std::shared_ptr<std::string> message;
};

// Emitted on server when an input seq was applied and Transform updated
struct MoveAppliedEvent : public Event {
  MoveAppliedEvent(clientid_t clientID, uint16_t seq, float x, float y,
                   uint8_t facing)
      : clientID(clientID), seq(seq), x(x), y(y), facing(facing) {}
  clientid_t clientID;
  uint16_t seq;
  float x, y;
  uint8_t facing;  // 0 left, 1 right
};

struct QuitEvent : public Event {};

#endif /* CORE_EVENT_ */
