#ifndef CORE_EVENTDISPATCHER_
#define CORE_EVENTDISPATCHER_

#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Core/Event.h"

/**
 * @brief A handle to an event subscription.
 * When the handle is destroyed, the subscription is automatically removed.
 */
class EventHandle {
public:
  EventHandle(class EventDispatcher *eventDispatcher, std::type_index typeIndex,
              size_t id);
  ~EventHandle();

private:
  class EventDispatcher *eventDispatcher;
  std::type_index typeIndex;
  size_t callbackID;
};

/**
 * @brief Manages event subscriptions and publications for immediate, synchronous communication.
 * @details Systems can subscribe to specific event types. When an event is published,
 *          the eventDispatcher immediately invokes all registered callback functions for that event type.
 *          Subscriptions are managed by EventHandle objects, which automatically unsubscribe
 *          upon destruction.
 */
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

  // 
  // Returns a 
  /**
   * @brief Subscribes a callback function to a specific event type.
   * 
   * @tparam EventType Event class to subscribe to.
   * @param callback Callback function when given event is published.
   * @return EventHandle handle that automatically unsubscribes when it goes out of scope.
   */
  template <typename EventType>
  std::unique_ptr<EventHandle> Subscribe(std::function<void(const EventType &)> callback) {
    CallbackID id = nextCallbackID++;
    listeners[typeid(EventType)].emplace_back(
        id, [cb = std::move(callback)](const Event &evt) {
          if (auto *e = dynamic_cast<const EventType *>(&evt)) {
            cb(*e);
          }
        });
    return std::make_unique<EventHandle>(this, typeid(EventType), id);
  }

  /**
   * @brief Publishes an event to all subscribed listeners immediately.
   * 
   * @param event Event to broadcast to all subscribers
   */
  void Publish(const Event &event) {
    auto it = listeners.find(typeid(event));
    if (it != listeners.end()) {
      for (const auto &pair : it->second) {
        pair.second(event);
      }
    }
  }

private:
/**
 * @brief Called by EventHandle destructor to remove the subscription.
 * 
 * @param ti typeid of given Event
 * @param id ID Callback function to remove
 */
  void Unsubscribe(const std::type_index &ti, CallbackID id);
};

#endif /* CORE_EVENTDISPATCHER_ */
