#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>
#include <utility>

template <typename T, size_t Size>
class RingBuffer {
 public:
  void push_back(T item) {
    buffer[lastIdx] = std::move(item);
    lastIdx = (lastIdx + 1) % Size;
    if (isFull) {
      startIdx = (startIdx + 1) % Size;
    } else if (lastIdx == startIdx) {
      isFull = true;
    }
  }

  T& front() {
    if (empty()) {
      throw std::out_of_range("front() called on empty RingBuffer");
    }
    return buffer[startIdx];
  }

  const T& front() const {
    if (empty()) {
      throw std::out_of_range("front() called on empty RingBuffer");
    }
    return buffer[startIdx];
  }

  T& back() {
    if (empty()) {
      throw std::out_of_range("back() called on empty RingBuffer");
    }
    return buffer[(lastIdx + Size - 1) % Size];
  }

  const T& back() const {
    if (empty()) {
      throw std::out_of_range("back() called on empty RingBuffer");
    }
    return buffer[(lastIdx + Size - 1) % Size];
  }

  void pop_front() {
    if (empty()) {
      return;
    }
    isFull = false;
    startIdx = (startIdx + 1) % Size;
  }

  bool empty() const { return !isFull && (lastIdx == startIdx); }

  size_t size() const {
    if (isFull) return Size;
    if (lastIdx >= startIdx) return lastIdx - startIdx;
    return Size + lastIdx - startIdx;
  }

  template <typename Predicate>
  void pop_front_while(Predicate pred) {
    while (!empty()) {
      if (pred(buffer[startIdx])) {
        pop_front();
      } else {
        break;
      }
    }
  }

  template <typename Func>
  void for_each(Func func) {
    if (empty()) return;

    size_t current = startIdx;
    while (current != lastIdx) {
      func(buffer[current]);
      current = (current + 1) % Size;
    }
  }

  template <typename Func>
  void for_each(Func func) const {
    if (empty()) return;

    size_t current = startIdx;
    while (current != lastIdx) {
      func(buffer[current]);
      current = (current + 1) % Size;
    }
  }

  const std::array<T, Size>& data() const { return buffer; }

  size_t getOldestIndex() const { return startIdx; }

 private:
  std::array<T, Size> buffer;
  size_t lastIdx = 0;
  size_t startIdx = 0;
  bool isFull = false;
};
