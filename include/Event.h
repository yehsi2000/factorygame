#ifndef EVENT_
#define EVENT_

#include <memory>

// --- Event-related structs ---
struct Event {
  virtual ~Event() = default;
};

struct StartInteractEvent : public Event {};

struct StopInteractEvent : public Event {};

struct XAxisEvent : public Event {
  XAxisEvent(float i) : val(i) {}
  float val;
};

struct YAxisEvent : public Event {
  YAxisEvent(float i) : val(i) {}
  float val;
};

struct QuitEvent : public Event {};

#endif /* EVENT_ */
