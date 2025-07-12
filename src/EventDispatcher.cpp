#include "EventDispatcher.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "CommandQueue.h"
#include "Event.h"

// --- EventHandle Implementation ---
EventHandle::EventHandle(EventDispatcher* edp, std::type_index ti,
                         CallbackID id)
    : dispatcher(edp), typeIndex(ti), callbackID(id) {}

EventHandle::~EventHandle() {
  if (dispatcher) {
    dispatcher->Unsubscribe(typeIndex, callbackID);
  }
}

EventHandle::EventHandle(EventHandle&& other) noexcept
    : dispatcher(other.dispatcher),
      typeIndex(other.typeIndex),
      callbackID(other.callbackID) {
  other.dispatcher = nullptr;
}

EventHandle& EventHandle::operator=(EventHandle&& other) noexcept {
  if (this != &other) {
    if (dispatcher) {
      dispatcher->Unsubscribe(typeIndex, callbackID);
    }
    dispatcher = other.dispatcher;
    typeIndex = other.typeIndex;
    callbackID = other.callbackID;
    other.dispatcher = nullptr;
  }
  return *this;
}

void EventDispatcher::Publish(const std::shared_ptr<const Event> e) {
  if (!commandQueue) return;
  
  commandQueue->Push(
      [this, e]() { this->ProcessEvent(*e); });
}

void EventDispatcher::ProcessEvent(const Event& event) {
  auto it = listeners.find(typeid(event));
  if (it != listeners.end()) {
    // 콜백 내부에서 unsubscribe 가능성 때문에 복사
    auto callbacks = it->second;
    for (const auto& [_, cb] : callbacks) {
      cb(event);
    }
  }
}

void EventDispatcher::Unsubscribe(const std::type_index& ti, CallbackID id) {
  auto it = listeners.find(ti);
  if (it != listeners.end()) {
    auto& vec = it->second;
    vec.erase(
        std::remove_if(vec.begin(), vec.end(),
                       [id](const auto& pair) { return pair.first == id; }),
        vec.end());
  }
}