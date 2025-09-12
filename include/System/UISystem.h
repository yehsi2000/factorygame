#ifndef SYSTEM_UISYSTEM_
#define SYSTEM_UISYSTEM_

#include <memory>
#include <list>
#include <string>

#include "Core/Entity.h"
#include "Core/Item.h"
#include "Core/SystemContext.h"


class EventHandle;

class UISystem {
  AssetManager* assetManager;
  EventDispatcher* eventDispatcher;
  Registry* registry;
  TimerManager* timerManager;
  World* world;

 public:
  UISystem(const SystemContext& context);
  ~UISystem();
  void Update();
  inline void ToggleInventory() { bIsShowingInventory = !bIsShowingInventory; }

 private:
  void ChatWindow();
  void ChatInput();
  void ItemDropBackground();
  void Inventory();
  void AssemblingMachineUI();
  void AssemblingMachineRecipeSelection(EntityID entity);
  void MiningDrillUI();
  void PushChat(std::shared_ptr<std::string> str);

  std::unique_ptr<EventHandle> showInventoryHandle;
  std::unique_ptr<EventHandle> showChatHandle;
  std::unique_ptr<EventHandle> newChatHandle;
  ItemPayload payload;
  bool bIsShowingInventory = false;
  bool bIsShowingChatInput = false;
  bool bDemoShow = true;
  std::shared_ptr<std::string> playerChat;
  std::list<std::string> chatLog;
};

#endif/* SYSTEM_UISYSTEM_ */
