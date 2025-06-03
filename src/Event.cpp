#include "Event.h"

void EventDispatcher::Dispatch(const Event& event) const{
    auto it = listeners.find(typeid(event));
    if (it != listeners.end()) {
        for (const auto& [_,cb] : it->second) {
            cb(event);
        }   
    }
}