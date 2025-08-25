#ifndef UTIL_ENTITYFACTORY_
#define UTIL_ENTITYFACTORY_

#include "Core/Entity.h"
#include "Core/Type.h"

class Registry;
class World;
struct SDL_Renderer;

namespace factory {
EntityID CreateAssemblingMachine(Registry* registry, World* world,
                                 SDL_Renderer* renderer, Vec2f worldPos);

EntityID CreateAssemblingMachine(Registry* registry, World* world,
                                 SDL_Renderer* renderer, Vec2 tileIndex);
EntityID CreateMiningDrill(Registry* registry, World* world,
                                 SDL_Renderer* renderer, Vec2f worldPos);

EntityID CreateMiningDrill(Registry* registry, World* world,
                                 SDL_Renderer* renderer, Vec2 tileIndex);
                                 
EntityID CreatePlayer(Registry *registry, SDL_Renderer *renderer,
                      Vec2f worldPos);
}  // namespace factory

#endif /* UTIL_ENTITYFACTORY_ */