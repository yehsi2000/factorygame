#ifndef EVENTDISPATCHER_
#define EVENTDISPATCHER_

#include <functional>
#include <iostream>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "CommandQueue.h"
#include "Event.h"


using CallbackID = std::size_t;

class EventDispatcher;

class EventHandle {
 public:
  EventHandle(EventDispatcher* edp, std::type_index ti, CallbackID id);
  ~EventHandle();

  EventHandle(const EventHandle&) = delete;
  EventHandle& operator=(const EventHandle&) = delete;
  EventHandle(EventHandle&& other) noexcept;
  EventHandle& operator=(EventHandle&& other) noexcept;

 private:
  EventDispatcher* dispatcher;
  std::type_index typeIndex;
  CallbackID callbackID;
};

class EventDispatcher {
  using Callback = std::function<void(const Event&)>;
  friend class EventHandle;

  std::unordered_map<std::type_index,
                     std::vector<std::pair<CallbackID, Callback>>>
      listeners;
  CommandQueue* commandQueue = nullptr;
  CallbackID nextCallbackID = 1;

  void ProcessEvent(const Event& event);

 public:
  EventDispatcher() = default;
  EventDispatcher(const EventDispatcher&) = delete;
  EventDispatcher& operator=(const EventDispatcher&) = delete;

  void Init(CommandQueue* cq) { commandQueue = cq; }

  template <typename EventType>
  EventHandle Subscribe(std::function<void(const EventType&)> callback) {
    CallbackID id = nextCallbackID++;
    listeners[typeid(EventType)].emplace_back(
        id, [cb = std::move(callback)](const Event& evt) {
          if (auto e = dynamic_cast<const EventType*>(&evt)) {
            cb(*e);
          }
        });
    return EventHandle(this, typeid(EventType), id);
  }

  void Publish(const std::shared_ptr<const Event> e);

 protected:
  void Unsubscribe(const std::type_index& ti, CallbackID id);
};

#endif /* EVENTDISPATCHER_ */
