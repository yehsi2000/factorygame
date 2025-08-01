#ifndef CORE_OBJECTPOOL_
#define CORE_OBJECTPOOL_

#include <functional>
#include <memory>
#include <queue>

template <typename T>
class ObjectPool {
 private:
  std::queue<std::unique_ptr<T>> pool;
  std::function<std::unique_ptr<T>()> createFunction;

 public:
  // Constructor that takes a function to create new objects
  ObjectPool(std::function<std::unique_ptr<T>()> createFunc)
      : createFunction(std::move(createFunc)) {}

  // Get an object from the pool or create a new one if pool is empty
  std::unique_ptr<T> Acquire() {
    if (pool.empty()) {
      return createFunction();
    }

    auto object = std::move(pool.front());
    pool.pop();
    return object;
  }

  // Return an object to the pool
  void Release(std::unique_ptr<T> object) {
    // Reset the object to a default state if needed
    // This depends on the specific object type
    pool.push(std::move(object));
  }

  // Get the current size of the pool
  size_t Size() const { return pool.size(); }

  // Clear all objects in the pool
  void Clear() {
    while (!pool.empty()) {
      pool.pop();
    }
  }

  // Pre-allocate objects in the pool
  void PreAllocate(size_t count) {
    for (size_t i = 0; i < count; ++i) {
      pool.push(createFunction());
    }
  }
};

#endif /* CORE_OBJECTPOOL_ */
