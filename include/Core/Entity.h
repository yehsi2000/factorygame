#pragma once

#include <cstdint>
#include <functional>
#include <limits>

struct Entity {
  using IdType = std::uint32_t;

  static constexpr IdType null_id = std::numeric_limits<IdType>::max();
  friend class Registry;

 private:
  IdType id;
  IdType generation;

 public:
  // WARNING : if entity is created beside registry, that could be problem
  // this is to make entity trivial structure
  constexpr Entity() = default;

  // Explicit constructor to create an entity with a specific ID.
  constexpr explicit Entity(IdType id) : id(id), generation(0) {}

  // Overload bool operator for easy validity checks (e.g., if (myEntity) { ...
  // })
  constexpr explicit operator bool() const { return id != null_id; }

  // Comparison operators
  constexpr bool operator==(const Entity& other) const noexcept {
    if (id == other.id && generation == other.generation) return true;
    return false;
  }

  constexpr bool operator!=(const Entity& other) const noexcept {
    return !(*this == other);
  }

  constexpr IdType Id() const { return id; }

  // Static function to get a null entity instance, for clarity.
  static constexpr Entity Null() { return Entity(null_id); }
};

static_assert(std::is_standard_layout_v<Entity>);

static_assert(std::is_trivially_copyable_v<Entity>);

static_assert(std::is_trivially_default_constructible_v<Entity>);

static_assert(std::is_trivially_destructible_v<Entity>);
