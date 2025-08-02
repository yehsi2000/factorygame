#ifndef CORE_EVENT_
#define CORE_EVENT_
#include <memory>

#include "Core/Entity.h"

struct Event {
  virtual ~Event() = default;
};

struct InteractEvent : public Event {
  InteractEvent(EntityID i) : instigator(i) {}
  EntityID instigator;
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
