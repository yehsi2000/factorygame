#ifndef SYSTEM_INPUTSYSTEM_
#define SYSTEM_INPUTSYSTEM_

#include <cstddef>
#include <unordered_map>

#include "Core/Item.h"
#include "Core/SystemContext.h"
#include "Core/Type.h"
#include "SDL.h"
#include "imgui.h"


enum class InputAction {
  StartInteraction,
  StopInteraction,
  Inventory,
  MouseDrop,
  Debug,
  Quit
};
enum class InputType { KEYBOARD, MOUSE };

struct KeyEvent {
  SDL_Scancode Scancode;
  SDL_EventType EventType;
  bool operator==(const KeyEvent& other) const {
    return Scancode == other.Scancode && EventType == other.EventType;
  }
};

struct KeyEventHasher {
  std::size_t operator()(const KeyEvent& k) const;
};

class InputSystem {
  std::unordered_map<KeyEvent, InputAction, KeyEventHasher> keyBindings;

  Registry* registry;
  EventDispatcher* eventDispatcher;
  World* world;
  EntityFactory* factory;
  TimerManager* timerManager;
  AssetManager* assetManager;
  SDL_Window* window;

  ImGuiIO& io;

  Vec2 screenSize;

  SDL_Event event;

  
  bool isDraggingOutside;

  // Building preview state
  bool isPreviewingBuilding;
  ItemID previewingItemID;
  EntityID previewEntity;

  double maxInteractionRadius = 200.0;

 public:
  InputSystem(const SystemContext& context, SDL_Window* window);
  ~InputSystem();
  void Update();

  void HandleInputAction(InputAction action, InputType type,
                         void* params = nullptr);
  void HandleInputAxis(const Uint8* keyState);

 private:
  void RegisterInputBindings();
  void CreatePreviewEntity(ItemID itemID);
  void DestroyPreviewEntity();
  void UpdatePreviewEntity();
};

#endif /* SYSTEM_INPUTSYSTEM_ */
