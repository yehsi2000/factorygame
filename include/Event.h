#ifndef EVENT_
#define EVENT_
#include <memory>

struct Event {
  virtual ~Event() = default;
  
  // Priority for events (lower values have higher priority)
  int priority = 0;
  
  // Constructor with priority
  Event(int prio = 0) : priority(prio) {}
};

// PriorityEvent class that extends Event with priority support
struct PriorityEvent : public Event {
  PriorityEvent(int prio = 0) : Event(prio) {}
};

struct InteractEvent : public Event {
  InteractEvent(int prio = 0) : Event(prio) {}
};

struct XAxisEvent : public Event {
  XAxisEvent(float i, int prio = 0) : Event(prio), val(i) {}
  float val;
};

struct YAxisEvent : public Event {
  YAxisEvent(float i, int prio = 0) : Event(prio), val(i) {}
  float val;
};

struct QuitEvent : public Event {
  QuitEvent(int prio = 0) : Event(prio) {}
};

// Utility functions for event prioritization
namespace EventPriority {
  // Priority levels
  constexpr int HIGHEST = -100;
  constexpr int HIGH = -50;
  constexpr int NORMAL = 0;
  constexpr int LOW = 50;
  constexpr int LOWEST = 100;
  
  // Helper function to create high priority events
  template<typename T, typename... Args>
  std::shared_ptr<T> CreateHighPriorityEvent(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)..., HIGH);
  }
  
  // Helper function to create highest priority events
  template<typename T, typename... Args>
  std::shared_ptr<T> CreateHighestPriorityEvent(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)..., HIGHEST);
  }
  
  // Helper function to create low priority events
  template<typename T, typename... Args>
  std::shared_ptr<T> CreateLowPriorityEvent(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)..., LOW);
  }
}

#endif /* EVENT_ */
