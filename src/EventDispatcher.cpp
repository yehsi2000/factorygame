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

bool EventDispatcher::PublishDelayed(const std::shared_ptr<const Event> e, float delaySeconds) {
  if (!commandQueue) return false;
  
  // Schedule the event with the TimerManager
  return timerManager.ScheduleEvent(e, delaySeconds);
}

void EventDispatcher::ProcessDelayedEvents() {
  ProcessDelayedEventsBatch(100); // Use batch processing with default batch size
}

void EventDispatcher::ProcessDelayedEventsBatch(size_t batchSize) {
  if (!commandQueue) return;
  
  // Extract ready events in batch mode and publish them through the normal event system
  auto readyEvents = timerManager.ExtractReadyEventsBatch(batchSize);
  
  // Publish each extracted event
  for (const auto& event : readyEvents) {
    Publish(event);
  }
}

TimerManagerMetrics EventDispatcher::GetMetrics() const {
  return timerManager.GetMetrics();
}

void EventDispatcher::ResetMetrics() {
  timerManager.ResetMetrics();
}

void EventDispatcher::SetMaxQueueSize(int maxSize) {
  timerManager.SetMaxQueueSize(maxSize);
}

void EventDispatcher::Publish(const std::shared_ptr<const Event> e) {
  if (!commandQueue) return;
  
  commandQueue->Push(
      [this, e]() { this->ProcessEvent(*e); });
}

void EventDispatcher::PublishHighPriority(const std::shared_ptr<const Event> e) {
  if (!commandQueue) return;
  
  // For high-priority events, we still use the normal queue but document that
  // they should be processed with higher urgency
  // In a more sophisticated implementation, we might use a priority queue
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