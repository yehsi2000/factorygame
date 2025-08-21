#ifndef CORE_EVENTDISPATCHER_
#define CORE_EVENTDISPATCHER_

#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Core/Event.h"

// A handle to an event subscription.
// When the handle is destroyed, the subscription is automatically removed.
class EventHandle {
public:
  EventHandle(class EventDispatcher *dispatcher, std::type_index typeIndex,
              size_t id);
  ~EventHandle();

  EventHandle(const EventHandle &) = delete;
  EventHandle &operator=(const EventHandle &) = delete;
  EventHandle(EventHandle &&other) noexcept;
  EventHandle &operator=(EventHandle &&other) noexcept;

private:
  class EventDispatcher *dispatcher;
  std::type_index typeIndex;
  size_t callbackID;
};

class EventDispatcher {
  friend class EventHandle;
  using Callback = std::function<void(const Event &)>;
  using CallbackID = size_t;

  std::unordered_map<std::type_index,
                     std::vector<std::pair<CallbackID, Callback>>>
      listeners;
  CallbackID nextCallbackID = 1;

public:
  EventDispatcher() = default;
  EventDispatcher(const EventDispatcher &) = delete;
  EventDispatcher &operator=(const EventDispatcher &) = delete;

  // Subscribes a callback function to a specific event type.
  // Returns a handle that automatically unsubscribes when it goes out of scope.
  template <typename EventType>
  EventHandle Subscribe(std::function<void(const EventType &)> callback) {
    CallbackID id = nextCallbackID++;
    listeners[typeid(EventType)].emplace_back(
        id, [cb = std::move(callback)](const Event &evt) {
          if (auto *e = dynamic_cast<const EventType *>(&evt)) {
            cb(*e);
          }
        });
    return EventHandle(this, typeid(EventType), id);
  }

  // Publishes an event to all subscribed listeners immediately.
  void Publish(const Event &event) {
    auto it = listeners.find(typeid(event));
    if (it != listeners.end()) {
      for (const auto &pair : it->second) {
        pair.second(event);
      }
    }
  }

private:
  // Called by EventHandle destructor to remove the subscription.
  void Unsubscribe(const std::type_index &ti, CallbackID id);
};

#endif /* CORE_EVENTDISPATCHER_ */
