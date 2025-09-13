#ifndef SYSTEM_UISYSTEM_
#define SYSTEM_UISYSTEM_

#include <list>
#include <memory>
#include <string>

#include "Core/Entity.h"
#include "Core/Item.h"
#include "Common.h"
#include "Core/SystemContext.h"


constexpr float ICONSPRITE_WIDTHF = static_cast<float>(ICONSPRITE_WIDTH);
constexpr float ICONSPRITE_HEIGHTF = static_cast<float>(ICONSPRITE_HEIGHT);

constexpr float uB0 = 0 / ICONSPRITE_WIDTHF;
constexpr float vB0 = 0.f;
constexpr float uB1 = ICONSIZE_BIG / ICONSPRITE_WIDTHF;
constexpr float vB1 = ICONSIZE_BIG / ICONSPRITE_HEIGHTF;

constexpr float uM0 = ICONSIZE_BIG / ICONSPRITE_WIDTHF;
constexpr float vM0 = 0.f;
constexpr float uM1 = ICONSIZE_MID_XSTART / ICONSPRITE_WIDTHF;
constexpr float vM1 = ICONSIZE_MID / ICONSPRITE_HEIGHTF;

constexpr float uS0 = ICONSIZE_MID_XSTART / ICONSPRITE_WIDTHF;
constexpr float vS0 = 0.f;
constexpr float uS1 = ICONSIZE_SMALL_XSTART / ICONSPRITE_WIDTHF;
constexpr float vS1 = ICONSIZE_SMALL / ICONSPRITE_HEIGHTF;

constexpr float uT0 = ICONSIZE_SMALL_XSTART / ICONSPRITE_WIDTHF;
constexpr float vT0 = 0.f;
constexpr float uT1 = ICONSIZE_TINY_XSTART / ICONSPRITE_WIDTHF;
constexpr float vT1 = ICONSIZE_TINY / ICONSPRITE_HEIGHTF;

constexpr float invNodeSize = 64.f;

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

#endif /* SYSTEM_UISYSTEM_ */
