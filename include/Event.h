#pragma once
#include <algorithm>
#include <exception>
#include <functional>
#include <iostream>
#include <typeindex>
#include <unordered_map>

#include "Entity.h"

using CallbackID = std::size_t;

struct Event {
  virtual ~Event() = default;
};

struct OreMinedEvent : public Event {
  EntityID playerID;
  int amount;
  OreMinedEvent(EntityID id, int amt) : playerID(id), amount(amt) {}
};


class EventDispatcher {
  using Callback = std::function<void(const Event&)>;
  std::unordered_map<std::type_index, std::vector<std::pair<CallbackID, Callback>>> listeners;
  CallbackID nextID = 1;

 public:
  EventDispatcher() = default;
  EventDispatcher(const EventDispatcher&) = delete;
  EventDispatcher& operator=(const EventDispatcher&) = delete;
  template <typename EventType>
  CallbackID Subscribe(std::function<void(const EventType&)> callback) {
    CallbackID id = nextID++;
    try {
      listeners[typeid(EventType)].push_back(
        {id, 
          [cb = std::move(callback)](const Event& evt) {
            auto e = dynamic_cast<const EventType&>(evt);
            cb(e);
          }
        }
      );
    } catch (std::bad_cast) {
      std::cout << "wrong event type is passed" << std::endl;
      return 0;
    }
    return id;
  }

  template <typename EventType>
  void Unsubscribe(CallbackID id) {
    auto& vec = listeners[typeid(EventType)];
    vec.erase(std::remove_if(vec.begin(), vec.end(), [id](const auto& pair) { return pair.first == id; }), vec.end());
  }

  void Dispatch(const Event& event) const;
};
