#include "Core/EventDispatcher.h"

EventHandle::EventHandle(EventDispatcher* eventDispatcher, std::type_index typeIndex,
                         size_t id)
    : eventDispatcher(eventDispatcher), typeIndex(typeIndex), callbackID(id) {}

EventHandle::~EventHandle() {
  if (eventDispatcher) {
    eventDispatcher->Unsubscribe(typeIndex, callbackID);
  }
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
