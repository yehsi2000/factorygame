#include <SDL.h>
#include <SDL_ttf.h>

#include <cassert>
#include <iostream>

#include "Components/BuildingComponent.h"
#include "Components/ChunkComponent.h"
#include "Components/InactiveComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TextComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Registry.h"
#include "Core/World.h"


// Mock SDL renderer and font for testing
class TestWorld {
private:
  SDL_Renderer *mockRenderer;
  TTF_Font *mockFont = nullptr;
  Registry *registry;
  World *world;

public:
  TestWorld() {
    registry = new Registry();
    registry->RegisterComponent<TransformComponent>();
    registry->RegisterComponent<BuildingComponent>();
    registry->RegisterComponent<SpriteComponent>();
    registry->RegisterComponent<ChunkComponent>();
    registry->RegisterComponent<ResourceNodeComponent>();
    registry->RegisterComponent<TextComponent>();
    registry->RegisterComponent<InactiveComponent>();
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_Window *window =
        SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_HIDDEN);
    mockRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    world = new World(mockRenderer, registry, mockFont);
  }

  bool test_tile_coordinate_conversion() {
    // Test world position to tile index conversion
    Vec2 tileIndex = world->GetTileIndexFromWorldPosition(
        64.0f, 64.0f); // TILE_PIXEL_SIZE = 64
    if (tileIndex.x != 1 || tileIndex.y != 1) {
      std::cerr << "World to tile conversion failed: expected (1,1), got ("
                << tileIndex.x << "," << tileIndex.y << ")" << std::endl;
      return false;
    }

    // Test negative coordinates
    tileIndex = world->GetTileIndexFromWorldPosition(-64.0f, -64.0f);
    if (tileIndex.x != -1 || tileIndex.y != -1) {
      std::cerr
          << "Negative coordinate conversion failed: expected (-1,-1), got ("
          << tileIndex.x << "," << tileIndex.y << ")" << std::endl;
      return false;
    }

    // Test fractional coordinates
    tileIndex = world->GetTileIndexFromWorldPosition(
        96.0f, 32.0f); // 1.5 tiles, 0.5 tiles
    if (tileIndex.x != 1 || tileIndex.y != 0) {
      std::cerr
          << "Fractional coordinate conversion failed: expected (1,0), got ("
          << tileIndex.x << "," << tileIndex.y << ")" << std::endl;
      return false;
    }

    return true;
  }

  bool test_chunk_loading_and_player_movement() {
    // Create a test player entity
    EntityID player = registry->CreateEntity();
    registry->AddComponent<TransformComponent>(player, Vec2f{0.0f, 0.0f});

    // Update world with player at origin
    world->Update(player);

    // Check that chunks around origin are loaded
    const auto &activeChunks = world->GetActiveChunks();

    // With view distance of 2, we should have chunks from (-2,-2) to (2,2)
    int expectedChunks = 5 * 5; // 5x5 grid
    if (activeChunks.size() != expectedChunks) {
      std::cerr << "Initial chunk loading failed: expected " << expectedChunks
                << " chunks, got " << activeChunks.size() << std::endl;
      return false;
    }

    // Check specific chunks exist
    ChunkCoord origin{0, 0};
    ChunkCoord corner{-2, -2};
    ChunkCoord farCorner{2, 2};

    if (activeChunks.find(origin) == activeChunks.end()) {
      std::cerr << "Origin chunk not loaded" << std::endl;
      return false;
    }

    if (activeChunks.find(corner) == activeChunks.end()) {
      std::cerr << "Corner chunk not loaded" << std::endl;
      return false;
    }

    // Move player far away to test chunk unloading
    auto &transform = registry->GetComponent<TransformComponent>(player);
    transform.position =
        Vec2f{CHUNK_WIDTH * TILE_PIXEL_SIZE * world->viewDistance * 10.f,
              CHUNK_HEIGHT * TILE_PIXEL_SIZE * world->viewDistance *
                  10.f}; // Far from origin

    world->Update(player);

    const auto &newActiveChunks = world->GetActiveChunks();

    // Origin chunk should no longer be active
    if (newActiveChunks.find(origin) != newActiveChunks.end()) {
      std::cerr << "Origin chunk not unloaded after player movement"
                << std::endl;
      return false;
    }

    return true;
  }

  bool test_building_placement() {
    // Test building placement validation

    // Create a test player to ensure chunks are loaded
    EntityID player = registry->CreateEntity();
    registry->AddComponent<TransformComponent>(player, Vec2f{64.0f, 64.0f});
    world->Update(player);

    // Test valid placement
    Vec2 tileIndex{1, 1};
    bool canPlace = world->CanPlaceBuilding(tileIndex, 2, 2);
    if (!canPlace) {
      std::cerr << "Valid building placement rejected" << std::endl;
      return false;
    }

    // Actually place the building
    EntityID building = registry->CreateEntity();
    registry->AddComponent<BuildingComponent>(building, BuildingComponent{});

    world->PlaceBuilding(building, tileIndex, 2, 2);

    // Test that the same location is now occupied
    bool canPlaceAgain = world->CanPlaceBuilding(tileIndex, 2, 2);
    if (canPlaceAgain) {
      std::cerr << "Occupied tiles allowed building placement" << std::endl;
      return false;
    }

    // Test overlapping placement
    Vec2 overlappingIndex{2, 2}; // Should overlap with existing building
    bool canPlaceOverlap = world->CanPlaceBuilding(overlappingIndex, 2, 2);
    if (canPlaceOverlap) {
      std::cerr << "Overlapping building placement allowed" << std::endl;
      return false;
    }

    // Test adjacent placement (should work)
    Vec2 adjacentIndex{3, 1}; // Next to existing building
    bool canPlaceAdjacent = world->CanPlaceBuilding(adjacentIndex, 1, 1);
    if (!canPlaceAdjacent) {
      std::cerr << "Adjacent building placement rejected" << std::endl;
      return false;
    }

    return true;
  }

  bool test_building_removal() {
    // Setup: place a building first
    EntityID player = registry->CreateEntity();
    registry->AddComponent<TransformComponent>(player, Vec2f{64.0f, 64.0f});
    world->Update(player);

    EntityID building = registry->CreateEntity();
    registry->AddComponent<BuildingComponent>(building, BuildingComponent{});

    Vec2 tileIndex{1, 1};
    world->PlaceBuilding(building, tileIndex, 2, 2);

    // Verify it's placed
    if (world->CanPlaceBuilding(tileIndex, 2, 2)) {
      std::cerr << "Building not properly placed before removal test"
                << std::endl;
      return false;
    }

    // Get the occupied tiles from building component
    auto &buildingComp = registry->GetComponent<BuildingComponent>(building);
    std::vector<Vec2> occupiedTiles = buildingComp.occupiedTiles;

    // Remove the building
    world->RemoveBuilding(building, occupiedTiles);

    // Verify the tiles are now free
    if (!world->CanPlaceBuilding(tileIndex, 2, 2)) {
      std::cerr << "Building removal failed: tiles still occupied" << std::endl;
      return false;
    }

    return true;
  }

  bool test_tile_access() {
    // Create player and load chunks
    EntityID player = registry->CreateEntity();
    registry->AddComponent<TransformComponent>(player, Vec2f{64.0f, 64.0f});
    world->Update(player);

    // Test getting tile at world position
    TileData *tile = world->GetTileAtWorldPosition(64.0f, 64.0f);
    if (!tile) {
      std::cerr << "Could not get tile at valid world position" << std::endl;
      return false;
    }

    // Test getting tile at tile index
    TileData *tileByIndex = world->GetTileAtTileIndex(1, 1);
    if (!tileByIndex) {
      std::cerr << "Could not get tile at valid tile index" << std::endl;
      return false;
    }

    // Should be the same tile
    if (tile != tileByIndex) {
      std::cerr << "World position and tile index should return same tile"
                << std::endl;
      return false;
    }

    // Test getting tile in unloaded chunk (should return nullptr)
    TileData *farTile = world->GetTileAtTileIndex(1000, 1000);
    if (farTile != nullptr) {
      std::cerr << "Getting tile in unloaded chunk should return nullptr"
                << std::endl;
      return false;
    }

    return true;
  }

  bool test_ore_generation_values() {
    // Test that ore amount values are properly set
    rsrc_amt_t minOre = world->GetMinironOreAmount();
    rsrc_amt_t maxOre = world->GetMaxironOreAmount();

    if (maxOre <= 0) {
      std::cerr << "Max ore amount should be positive, got " << maxOre
                << std::endl;
      return false;
    }

    if (minOre < 0 || minOre > maxOre) {
      std::cerr << "Min ore amount should be between 0 and max, got " << minOre
                << " (max: " << maxOre << ")" << std::endl;
      return false;
    }

    return true;
  }
};

int main(int argc, char *argv[]) {
  bool all_passed = true;
  TestWorld testWorld;

  std::cout << "Running World tests..." << std::endl;

  if (!testWorld.test_tile_coordinate_conversion()) {
    std::cerr << "Tile coordinate conversion test failed" << std::endl;
    all_passed = false;
  } else {
    std::cout << "Tile coordinate conversion test passed" << std::endl;
  }

  if (!testWorld.test_chunk_loading_and_player_movement()) {
    std::cerr << "Chunk loading and player movement test failed" << std::endl;
    all_passed = false;
  } else {
    std::cout << "Chunk loading and player movement test passed" << std::endl;
  }

  if (!testWorld.test_building_placement()) {
    std::cerr << "Building placement test failed" << std::endl;
    all_passed = false;
  } else {
    std::cout << "Building placement test passed" << std::endl;
  }

  if (!testWorld.test_building_removal()) {
    std::cerr << "Building removal test failed" << std::endl;
    all_passed = false;
  } else {
    std::cout << "Building removal test passed" << std::endl;
  }

  if (!testWorld.test_tile_access()) {
    std::cerr << "Tile access test failed" << std::endl;
    all_passed = false;
  } else {
    std::cout << "Tile access test passed" << std::endl;
  }

  if (!testWorld.test_ore_generation_values()) {
    std::cerr << "Ore generation values test failed" << std::endl;
    all_passed = false;
  } else {
    std::cout << "Ore generation values test passed" << std::endl;
  }

  if (all_passed) {
    std::cout << "\n All World tests passed!" << std::endl;
    return 0;
  } else {
    std::cerr << "\n Some World tests failed!" << std::endl;
    return 1;
  }
}