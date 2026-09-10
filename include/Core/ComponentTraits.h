#pragma once

#include <type_traits>

struct AssemblingMachineComponent;
struct BuildingComponent;
struct InventoryComponent;

/**
 * @brief Storage policy for ECS component types.
 * @details Components are required to be trivially copyable and trivially
 * destructible by default. Specialize this trait only for components that
 * intentionally own non-trivial state such as dynamic containers.
 */
template <typename T>
struct ComponentTraits {
  static constexpr bool allowNonTrivial = false;
};

template <>
struct ComponentTraits<AssemblingMachineComponent> {
  static constexpr bool allowNonTrivial = true;
};

template <>
struct ComponentTraits<BuildingComponent> {
  static constexpr bool allowNonTrivial = true;
};

template <>
struct ComponentTraits<InventoryComponent> {
  static constexpr bool allowNonTrivial = true;
};

template <typename T>
inline constexpr bool is_component_storage_compatible_v =
    ComponentTraits<T>::allowNonTrivial ||
    (std::is_trivially_copyable_v<T> &&
     std::is_trivially_destructible_v<T>);
