#pragma once
 
#include <array>
#include <stdexcept>
#include <cstddef>

template <typename T, size_t Size>
class RingBuffer {
 public:
  void push_back(T item) {
    buffer[headIdx] = std::move(item);
    headIdx = (headIdx + 1) % Size;
    if (isFull) {
      tailIdx = (tailIdx + 1) % Size;
    } else if (headIdx == tailIdx) {
      isFull = true;
    }
  }

  T& front() {
    if (empty()) {
      throw std::out_of_range("front() called on empty RingBuffer");
    }
    return buffer[tailIdx];
  }

  const T& front() const {
    if (empty()) {
      throw std::out_of_range("front() called on empty RingBuffer");
    }
    return buffer[tailIdx];
  }

  void pop_front() {
    if (empty()) {
      return;
    }
    isFull = false;
    tailIdx = (tailIdx + 1) % Size;
  }

  bool empty() const { return !isFull && (headIdx == tailIdx); }

  size_t size() const {
    if (isFull) return Size;
    if (headIdx >= tailIdx) return headIdx - tailIdx;
    return Size + headIdx - tailIdx;
  }

  template <typename Predicate>
  void pop_front_while(Predicate pred) {
    while (!empty()) {
      if (pred(buffer[tailIdx])) {
        pop_front();
      } else {
        break;
      }
    }
  }

  template <typename Func>
  void for_each(Func func) {
    if (empty()) return;

    size_t current = tailIdx;
    while (current != headIdx) {
      func(buffer[current]);
      current = (current + 1) % Size;
    }
  }

  const std::array<T, Size> data() const{
    return buffer;
  }

 private:
  std::array<T, Size> buffer;
  size_t headIdx = 0;
  size_t tailIdx = 0;
  bool isFull = false;
};
