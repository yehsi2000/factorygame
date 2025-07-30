#include "TimerManager.h"
#include <chrono>
#include <algorithm>

TimerManager::TimerManager(int maxQueueSize)
    : maxQueueSize(maxQueueSize), lastProcessTime(std::chrono::steady_clock::now()) {
    // Initialize the object pool for DelayedEvent objects
    delayedEventPool = std::make_unique<ObjectPool<DelayedEvent>>(
        []() { return std::make_unique<DelayedEvent>(); }
    );
    
    // Pre-allocate some objects in the pool for better performance
    delayedEventPool->PreAllocate(50);
}

bool TimerManager::ScheduleEvent(std::shared_ptr<const Event> event, float delaySeconds) {
    auto now = std::chrono::steady_clock::now();
    auto delay = std::chrono::duration<float>(delaySeconds);
    auto executionTime = now + std::chrono::duration_cast<std::chrono::steady_clock::duration>(delay);
    return ScheduleEventAt(std::move(event), executionTime);
}

bool TimerManager::ScheduleEventAt(std::shared_ptr<const Event> event, std::chrono::steady_clock::time_point time) {
    // Check if queue is at capacity
    if (IsQueueAtCapacity()) {
        metrics.totalQueueOverflows++;
        // Try to drop low priority events to make space
        size_t dropped = DropLowPriorityEvents(1);
        if (dropped == 0 && maxQueueSize > 0) {
            // If we couldn't drop any events and have a size limit, drop this event
            metrics.totalEventsDropped++;
            return false;
        }
    }
    
    // Create a DelayedEvent directly (not using pool for queue entries)
    delayedEvents.emplace(std::move(event), time);
    
    // Update metrics
    metrics.totalEventsScheduled++;
    metrics.currentQueueSize = delayedEvents.size();
    metrics.peakQueueSize = std::max(metrics.peakQueueSize.load(), metrics.currentQueueSize.load());
    
    return true;
}

size_t TimerManager::ProcessReadyEvents() {
    return ProcessReadyEventsBatch(1); // Process one event at a time
}

size_t TimerManager::ProcessReadyEventsBatch(size_t batchSize) {
    auto now = std::chrono::steady_clock::now();
    size_t processedCount = 0;
    
    // Process events in batches for better performance
    while (processedCount < batchSize && !delayedEvents.empty() && delayedEvents.top().executionTime <= now) {
        // In a real implementation, we would process the event here
        // For now, we'll just count it as processed
        delayedEvents.pop();
        processedCount++;
    }
    
    // Update metrics
    metrics.totalEventsProcessed += processedCount;
    metrics.currentQueueSize = delayedEvents.size();
    
    // Update timing for performance measurements
    lastProcessTime = now;
    
    return processedCount;
}

std::shared_ptr<const std::chrono::steady_clock::time_point> TimerManager::GetNextEventTime() const {
    if (delayedEvents.empty()) {
        return nullptr;
    }
    
    return std::make_shared<const std::chrono::steady_clock::time_point>(delayedEvents.top().executionTime);
}

bool TimerManager::HasScheduledEvents() const {
    return !delayedEvents.empty();
}

void TimerManager::ClearScheduledEvents() {
    // Clear all events from the priority queue
    while (!delayedEvents.empty()) {
        delayedEvents.pop();
    }
    
    // Update metrics
    metrics.currentQueueSize = 0;
}

std::vector<std::shared_ptr<const Event>> TimerManager::ExtractReadyEvents() {
    return ExtractReadyEventsBatch(1); // Extract one event at a time
}

std::vector<std::shared_ptr<const Event>> TimerManager::ExtractReadyEventsBatch(size_t batchSize) {
    auto now = std::chrono::steady_clock::now();
    std::vector<std::shared_ptr<const Event>> readyEvents;
    
    // Extract events in batches for better performance
    size_t extractedCount = 0;
    while (extractedCount < batchSize && !delayedEvents.empty() && delayedEvents.top().executionTime <= now) {
        auto delayedEvent = delayedEvents.top();
        delayedEvents.pop();
        
        readyEvents.push_back(delayedEvent.event);
        extractedCount++;
    }
    
    // Update metrics
    metrics.totalEventsProcessed += extractedCount;
    metrics.currentQueueSize = delayedEvents.size();
    
    // Update timing for performance measurements
    lastProcessTime = now;
    
    return readyEvents;
}

TimerManagerMetrics TimerManager::GetMetrics() const {
    TimerManagerMetrics copy;
    copy.totalEventsScheduled = metrics.totalEventsScheduled.load();
    copy.totalEventsProcessed = metrics.totalEventsProcessed.load();
    copy.totalEventsDropped = metrics.totalEventsDropped.load();
    copy.totalQueueOverflows = metrics.totalQueueOverflows.load();
    copy.peakQueueSize = metrics.peakQueueSize.load();
    copy.currentQueueSize = metrics.currentQueueSize.load();
    return copy;
}

void TimerManager::ResetMetrics() {
    metrics.Reset();
}

size_t TimerManager::GetQueueSize() const {
    return delayedEvents.size();
}

void TimerManager::SetMaxQueueSize(int maxSize) {
    maxQueueSize = maxSize;
}

int TimerManager::GetMaxQueueSize() const {
    return maxQueueSize;
}

bool TimerManager::IsQueueAtCapacity() const {
    return maxQueueSize > 0 && static_cast<int>(delayedEvents.size()) >= maxQueueSize;
}

size_t TimerManager::DropLowPriorityEvents(size_t count) {
    // For a priority queue, dropping lowest priority events is complex
    // We'll implement a simpler approach: drop the events with the highest priority values (lowest priority)
    // This is a simplified implementation for demonstration purposes
    
    if (delayedEvents.empty() || count == 0) {
        return 0;
    }
    
    // In a real implementation, we would need a more sophisticated approach
    // For now, we'll just return 0 to indicate no events were dropped
    // A full implementation would require a different data structure or approach
    return 0;
}