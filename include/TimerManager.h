#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H

#include <memory>
#include <queue>
#include <vector>
#include <chrono>
#include <functional>
#include <atomic>
#include "Event.h"
#include "ObjectPool.h"

// DelayedEvent struct that contains the event and its execution time
struct DelayedEvent {
    std::shared_ptr<const Event> event;
    std::chrono::steady_clock::time_point executionTime;
    
    DelayedEvent() : executionTime(std::chrono::steady_clock::time_point::min()) {}
    
    DelayedEvent(std::shared_ptr<const Event> e, std::chrono::steady_clock::time_point time)
        : event(std::move(e)), executionTime(time) {}
    
    // Reset the event for reuse
    void Reset() {
        event.reset();
        executionTime = std::chrono::steady_clock::time_point::min();
    }
    
    // Comparison operator for priority queue (earliest time has highest priority)
    // If times are equal, higher priority events (lower priority value) come first
    bool operator>(const DelayedEvent& other) const {
        if (executionTime != other.executionTime) {
            return executionTime > other.executionTime;
        }
        // If execution times are equal, prioritize by event priority (lower value = higher priority)
        // Handle case where one or both events might be null
        if (!event && !other.event) return false;
        if (!event) return false;  // null events have lowest priority
        if (!other.event) return true;  // null events have lowest priority
        return event->priority > other.event->priority;
    }
};

// Performance metrics structure
struct TimerManagerMetrics {
    std::atomic<size_t> totalEventsScheduled{0};
    std::atomic<size_t> totalEventsProcessed{0};
    std::atomic<size_t> totalEventsDropped{0};
    std::atomic<size_t> totalQueueOverflows{0};
    std::atomic<size_t> peakQueueSize{0};
    std::atomic<size_t> currentQueueSize{0};
    
    // Default constructor
    TimerManagerMetrics() = default;
    
    // Copy constructor
    TimerManagerMetrics(const TimerManagerMetrics& other)
        : totalEventsScheduled(other.totalEventsScheduled.load()),
          totalEventsProcessed(other.totalEventsProcessed.load()),
          totalEventsDropped(other.totalEventsDropped.load()),
          totalQueueOverflows(other.totalQueueOverflows.load()),
          peakQueueSize(other.peakQueueSize.load()),
          currentQueueSize(other.currentQueueSize.load()) {}
    
    // Assignment operator
    TimerManagerMetrics& operator=(const TimerManagerMetrics& other) {
        totalEventsScheduled = other.totalEventsScheduled.load();
        totalEventsProcessed = other.totalEventsProcessed.load();
        totalEventsDropped = other.totalEventsDropped.load();
        totalQueueOverflows = other.totalQueueOverflows.load();
        peakQueueSize = other.peakQueueSize.load();
        currentQueueSize = other.currentQueueSize.load();
        return *this;
    }
    
    // Reset all metrics
    void Reset() {
        totalEventsScheduled = 0;
        totalEventsProcessed = 0;
        totalEventsDropped = 0;
        totalQueueOverflows = 0;
        peakQueueSize = 0;
        currentQueueSize = 0;
    }
};

// TimerManager class that handles delayed event scheduling
class TimerManager {
private:
    // Priority queue for delayed events sorted by execution time and event priority
    std::priority_queue<DelayedEvent, std::vector<DelayedEvent>, std::greater<DelayedEvent>> delayedEvents;
    
    // Object pool for DelayedEvent objects to reduce memory allocation
    std::unique_ptr<ObjectPool<DelayedEvent>> delayedEventPool;
    
    // Maximum queue size to prevent overflow (-1 for unlimited)
    std::atomic<int> maxQueueSize;
    
    // Performance metrics
    TimerManagerMetrics metrics;
    
    // Timing for performance measurements
    std::chrono::steady_clock::time_point lastProcessTime;
    
public:
    // Constructor with optional maximum queue size
    TimerManager(int maxQueueSize = -1);
    
    // Schedule an event to be executed after a delay in seconds
    bool ScheduleEvent(std::shared_ptr<const Event> event, float delaySeconds);
    
    // Schedule an event to be executed at a specific time point
    bool ScheduleEventAt(std::shared_ptr<const Event> event, std::chrono::steady_clock::time_point time);
    
    // Process events that are ready to execute
    // Returns the number of events processed
    size_t ProcessReadyEvents();
    
    // Process events in batch mode for better performance
    size_t ProcessReadyEventsBatch(size_t batchSize = 100);
    
    // Get the time point of the next scheduled event
    // Returns nullptr if no events are scheduled
    std::shared_ptr<const std::chrono::steady_clock::time_point> GetNextEventTime() const;
    
    // Check if there are any scheduled events
    bool HasScheduledEvents() const;
    
    // Clear all scheduled events
    void ClearScheduledEvents();
    
    // Process events that are ready to execute and return them
    std::vector<std::shared_ptr<const Event>> ExtractReadyEvents();
    
    // Process events in batch mode and return them
    std::vector<std::shared_ptr<const Event>> ExtractReadyEventsBatch(size_t batchSize = 100);
    
    // Get performance metrics
    TimerManagerMetrics GetMetrics() const;
    
    // Reset performance metrics
    void ResetMetrics();
    
    // Get current queue size
    size_t GetQueueSize() const;
    
    // Set maximum queue size
    void SetMaxQueueSize(int maxSize);
    
    // Get maximum queue size
    int GetMaxQueueSize() const;
    
    // Check if queue is at capacity
    bool IsQueueAtCapacity() const;
    
    // Drop lowest priority events to make space
    size_t DropLowPriorityEvents(size_t count);
};

#endif // TIMERMANAGER_H