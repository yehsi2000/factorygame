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
  ObjectPool(std::function<std::unique_ptr<T>()> createFunc)
      : createFunction(std::move(createFunc)) {}

  /**
   * @brief Get an object from the pool or create a new one if pool is empty
   * 
   * @return std::unique_ptr<T> Object inside the pool
   */
  std::unique_ptr<T> Acquire() {
    if (pool.empty()) {
      return createFunction();
    }

    auto object = std::move(pool.front());
    pool.pop();
    return object;
  }

  /**
   * @brief Return an object to the pool
   * @details Reset the object to a default state if needed. This depends on the specific object type.
   * @param object Returned Object
   */
  void Release(std::unique_ptr<T> object) {
    pool.push(std::move(object));
  }

  size_t Size() const { return pool.size(); }

  void Clear() {
    while (!pool.empty()) {
      pool.pop();
    }
  }

  
  /**
   * @brief Pre-warm the pool
   * 
   * @param count Desired object pool size
   */
  void PreAllocate(size_t count) {
    for (size_t i = 0; i < count; ++i) {
      pool.push(createFunction());
    }
  }
};

#endif /* CORE_OBJECTPOOL_ */
