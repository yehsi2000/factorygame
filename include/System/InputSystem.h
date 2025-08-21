#ifndef SYSTEM_INPUTSYSTEM_
#define SYSTEM_INPUTSYSTEM_

#include <cstddef>
#include <unordered_map>

#include "SDL.h"
#include "imgui.h"
#include "Core/Item.h"
#include "Core/Entity.h"

enum class InputAction { StartInteraction, StopInteraction, Inventory, MouseDrop, Quit };
enum class InputType { KEYBOARD, MOUSE };

class GEngine;

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
  GEngine* engine;
  ImGuiIO& io;
  double maxInteractionRadius = 200.0;
  bool isDraggingOutside;
  
  // Building preview state
  bool isPreviewingBuilding;
  ItemID previewingItemID;
  EntityID previewEntity;

 public:
  InputSystem(GEngine* e);
  void Update();
  void InitInputSystem();

  void HandleInputAction(InputAction action, InputType type, void* params = nullptr);
  void HandleInputAxis(const Uint8* keyState);

 private:
  void RegisterInputBindings();
  void CreatePreviewEntity(ItemID itemID);
  void DestroyPreviewEntity();
  void UpdatePreviewEntity();
};

#endif /* SYSTEM_INPUTSYSTEM_ */
