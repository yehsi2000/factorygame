#ifndef COMPONENTS_BUILDINGPREVIEWCOMPONENT_
#define COMPONENTS_BUILDINGPREVIEWCOMPONENT_

#include "Core/Item.h"

struct BuildingPreviewComponent {
  ItemID itemID;
  int width;
  int height;
  
  constexpr BuildingPreviewComponent() 
    : itemID(ItemID::None), width(1), height(1) {}
    
  constexpr BuildingPreviewComponent(ItemID id, int w, int h) 
    : itemID(id), width(w), height(h) {}
};

#endif /* COMPONENTS_BUILDINGPREVIEWCOMPONENT_ */