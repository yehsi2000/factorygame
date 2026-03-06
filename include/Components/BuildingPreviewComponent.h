#pragma once

#include "Core/Item.h"

struct BuildingPreviewComponent {
  ItemID itemID;
  int width;
  int height;

  constexpr BuildingPreviewComponent() = default;

  constexpr BuildingPreviewComponent(ItemID id, int w, int h)
      : itemID(id), width(w), height(h) {}
};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<BuildingPreviewComponent> == true);
static_assert(std::is_trivially_destructible_v<BuildingPreviewComponent> == true);
#endif