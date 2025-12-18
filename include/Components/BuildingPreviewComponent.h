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
