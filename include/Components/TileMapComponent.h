#ifndef COMPONENTS_TILEMAPCOMPONENT_
#define COMPONENTS_TILEMAPCOMPONENT_

#include <map>
#include <string>

#include "SDL.h"

struct TileMapComponent {
  std::map<std::string, SDL_Surface*> tileImageMap;
};

#endif /* COMPONENTS_TILEMAPCOMPONENT_ */
