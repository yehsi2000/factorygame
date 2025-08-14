#include "Core/EventDispatcher.h"

EventHandle::EventHandle(EventDispatcher* dispatcher, std::type_index typeIndex,
                         size_t id)
    : dispatcher(dispatcher), typeIndex(typeIndex), callbackID(id) {}

EventHandle::~EventHandle() {
  if (dispatcher) {
    dispatcher->Unsubscribe(typeIndex, callbackID);
  }
}

EventHandle::EventHandle(EventHandle&& other) noexcept
    : dispatcher(other.dispatcher),
      typeIndex(other.typeIndex),
      callbackID(other.callbackID) {
  other.dispatcher = nullptr;  // Invalidate the other handle
}

EventHandle& EventHandle::operator=(EventHandle&& other) noexcept {
  if (this != &other) {
    if (dispatcher) {
      dispatcher->Unsubscribe(typeIndex, callbackID);
    }
    dispatcher = other.dispatcher;
    typeIndex = other.typeIndex;
    callbackID = other.callbackID;
    other.dispatcher = nullptr;  // Invalidate the other handle
  }
  return *this;
}

// --- EventDispatcher Implementation ---

void EventDispatcher::Unsubscribe(const std::type_index& ti, CallbackID id) {
  auto it = listeners.find(ti);
  if (it != listeners.end()) {
    auto& callbacks = it->second;
    for (auto vec_it = callbacks.begin(); vec_it != callbacks.end(); ++vec_it) {
      if (vec_it->first == id) {
        callbacks.erase(vec_it);
        break;
      }
    }
  }
}
