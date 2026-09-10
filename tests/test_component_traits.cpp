#include <type_traits>
#include <vector>

#include "Components/AssemblingMachineComponent.h"
#include "Components/BuildingComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/TransformComponent.h"
#include "Core/ComponentTraits.h"

namespace {

struct UnapprovedNonTrivialComponent {
  std::vector<int> values;
};

static_assert(std::is_trivially_copyable_v<TransformComponent>);
static_assert(std::is_trivially_destructible_v<TransformComponent>);
static_assert(!ComponentTraits<TransformComponent>::allowNonTrivial);
static_assert(is_component_storage_compatible_v<TransformComponent>);

static_assert(!std::is_trivially_copyable_v<BuildingComponent>);
static_assert(!std::is_trivially_destructible_v<BuildingComponent>);
static_assert(ComponentTraits<BuildingComponent>::allowNonTrivial);
static_assert(is_component_storage_compatible_v<BuildingComponent>);

static_assert(!std::is_trivially_copyable_v<InventoryComponent>);
static_assert(!std::is_trivially_destructible_v<InventoryComponent>);
static_assert(ComponentTraits<InventoryComponent>::allowNonTrivial);
static_assert(is_component_storage_compatible_v<InventoryComponent>);

static_assert(!std::is_trivially_copyable_v<AssemblingMachineComponent>);
static_assert(!std::is_trivially_destructible_v<AssemblingMachineComponent>);
static_assert(ComponentTraits<AssemblingMachineComponent>::allowNonTrivial);
static_assert(is_component_storage_compatible_v<AssemblingMachineComponent>);

static_assert(
    !ComponentTraits<UnapprovedNonTrivialComponent>::allowNonTrivial);
static_assert(
    !is_component_storage_compatible_v<UnapprovedNonTrivialComponent>);

}  // namespace

int main() { return 0; }
