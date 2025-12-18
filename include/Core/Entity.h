#pragma once

#include <cstdint>
#include <functional>
#include <limits>

struct Entity
{
    using IdType = std::uint64_t;

    static constexpr IdType null_id = std::numeric_limits<IdType>::max();

private:
    IdType id;

public:
    // WARNING : if entity is created beside registry, that could be problem
    // this is to make entity trivial structure
    constexpr Entity() = default;

    // Explicit constructor to create an entity with a specific ID.
    constexpr explicit Entity(IdType id) : id(id) {}

    // Check if the entity is valid (i.e., not null).
    constexpr bool IsValid() const { return id != null_id; }

    // Overload bool operator for easy validity checks (e.g., if (myEntity) { ... })
    constexpr explicit operator bool() const { return IsValid(); }

    // Explicit conversion to the underlying type.
    constexpr explicit operator IdType() const { return id; }

    // Comparison operators
    constexpr bool operator==(const Entity& other) const { return id == other.id; }
    constexpr bool operator!=(const Entity& other) const { return id != other.id; }
    constexpr bool operator<(const Entity& other) const { return id < other.id; }

    // Static function to get a null entity instance, for clarity.
    static constexpr Entity Null() { return Entity(null_id); }
};

// Hash function for using Entity as a key in unordered maps/sets
namespace std {
    template <>
    struct hash<Entity>
    {
        std::size_t operator()(const Entity& e) const
        {
            return std::hash<Entity::IdType>()(static_cast<Entity::IdType>(e));
        }
    };
}

static_assert(std::is_standard_layout_v<Entity>);

static_assert(std::is_trivially_copyable_v<Entity>);

static_assert(std::is_trivially_default_constructible_v<Entity>);

static_assert(std::is_trivially_destructible_v<Entity>);
