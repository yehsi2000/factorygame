#pragma once
#include <algorithm>
#include <functional>
#include <iostream>
#include <typeindex>
#include <unordered_map>

using CallbackID = std::size_t;

struct Event {
  virtual ~Event() = default;
};

struct StartInteractEvent : public Event {};

struct StopInteractEvent : public Event {};

class EventDispatcher {
  using Callback = std::function<void(const Event&)>;
  std::unordered_map<std::type_index,
                     std::vector<std::pair<CallbackID, Callback>>>
      listeners;
  CallbackID nextCallbackID = 1;

 public:
  EventDispatcher() = default;
  EventDispatcher(const EventDispatcher&) = delete;
  EventDispatcher& operator=(const EventDispatcher&) = delete;

  template <typename EventType>
  CallbackID Subscribe(std::function<void(const EventType&)> callback) {
    CallbackID id = nextCallbackID++;
    listeners[typeid(EventType)].emplace_back(
        id, [cb = std::move(callback)](const Event& evt) {
          if (auto e = dynamic_cast<const EventType*>(&evt)) {
            cb(*e);
          } else {
            std::cerr << "[EventDispatcher] Bad cast on event dispatch\n";
          }
        });
    return id;
  }

  void Unsubscribe(const std::type_index& ti, CallbackID id) {
    auto& vec = listeners[ti];
    vec.erase(
        std::remove_if(vec.begin(), vec.end(),
                       [id](const auto& pair) { return pair.first == id; }),
        vec.end());
  }

  void Dispatch(const Event& event) const {
    auto it = listeners.find(typeid(event));
    if (it != listeners.end()) {
      auto callbacks = it->second;
      for (const auto& [_, cb] : callbacks) {
        cb(event);
      }
    }
  }
};
