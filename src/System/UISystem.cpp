#include "System/UISystem.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <list>
#include <string>
#include <utility>

#include "Common.h"
#include "Components/AssemblingMachineComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/MiningDrillComponent.h"
#include "Core/AssetManager.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/Item.h"
#include "Core/Recipe.h"
#include "Core/Registry.h"
#include "Core/World.h"
#include "System/AssemblingMachineSystem.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "imgui_stdlib.h"


#define TEXTLATER

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

UISystem::UISystem(const SystemContext &context)
    : assetManager(context.assetManager),
      eventDispatcher(context.eventDispatcher),
      registry(context.registry),
      world(context.world) {
  showInventoryHandle = eventDispatcher->Subscribe<ToggleInventoryEvent>(
      [this](ToggleInventoryEvent e) {
        bIsShowingInventory = !bIsShowingInventory;
      });
  newChatHandle = eventDispatcher->Subscribe<NewChatEvent>(
      [this](NewChatEvent e) { PushChat(std::move(e.message)); });
  showChatHandle = eventDispatcher->Subscribe<ToggleChatInputEvent>(
      [this](ToggleChatInputEvent e) {
        if (!bIsShowingChatInput) {
          SDL_StartTextInput();
          bIsShowingChatInput = true;
          ImGui::SetKeyboardFocusHere(0);
        }
      });
  chatLog = std::list<std::string>();
  playerChat = std::make_shared<std::string>("");
}

void UISystem::PushChat(std::shared_ptr<std::string> str) {
  chatLog.push_back(*str.get());
  if (chatLog.size() > 10) chatLog.pop_front();
}

void UISystem::Update() {
  ItemDropBackground();
  ChatWindow();
  if (bDemoShow) {
    ImGui::ShowDemoWindow(&bDemoShow);
  }
  if (bIsShowingInventory) Inventory();
  if (bIsShowingChatInput) {
    ChatInput();
  }
  // SDL_StopTextInput();
  AssemblingMachineUI();
  MiningDrillUI();
}

void UISystem::ChatInput() {
  const ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImVec2 pos = viewport->WorkSize;
  ImVec2 size = ImVec2{viewport->WorkSize.x / 3.f, 30};
  ImGui::SetNextWindowSize(size);
  ImGui::SetNextWindowPos(ImVec2{pos.x / 2 - size.x / 2, pos.y - 50});
  ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});

  ImGui::Begin("##ChatInput", nullptr, flags);
  if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
  ImGui::SetNextItemWidth(size.x);
  ImGui::InputText("##ChatInputText", playerChat.get());
  if (ImGui::IsItemFocused()) {
    if (ImGui::IsKeyPressed(ImGuiKey_Enter, false)) {
      bIsShowingChatInput = false;
      eventDispatcher->Publish(SendChatEvent(playerChat));
      PushChat(playerChat);
      playerChat->clear();
      SDL_StopTextInput();
    }
  }
  ImGui::End();
  ImGui::PopStyleVar();
}

void UISystem::ChatWindow() {
  static int location = -1;
  ImGuiIO &io = ImGui::GetIO();
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
      ImGuiWindowFlags_NoNav;
  ImGui::SetNextWindowBgAlpha(0.35f);
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 1.f, 0.f, 1.f));
  ImGui::PushFont(nullptr, 16);
  if (ImGui::Begin("##ChatWindow", nullptr, window_flags)) {
    for (std::string_view chat : chatLog) {
      ImGui::Text(chat.data());
      ImGui::Separator();
    }
  }
  ImGui::PopFont();
  ImGui::PopStyleColor();
  ImGui::End();
}

void UISystem::ItemDropBackground() {
  const ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNav;

  if (ImGui::GetDragDropPayload() == nullptr) {
    flags |= ImGuiWindowFlags_NoInputs;
  }

  ImGui::PushStyleColor(ImGuiCol_DragDropTarget,
                        ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});

  ImGui::Begin("##GameWorldDropTarget", nullptr, flags);

  ImGui::Dummy(ImGui::GetContentRegionAvail());

  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("DND_ITEM")) {
      IM_ASSERT(payload->DataSize == sizeof(ItemPayload));
      ItemPayload *item_payload = static_cast<ItemPayload *>(payload->Data);

      eventDispatcher->Publish(ItemDropInWorldEvent{*item_payload});
    }
    ImGui::EndDragDropTarget();
  }

  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
  ImGui::End();
}

void UISystem::Inventory() {
  InventoryComponent &invComp =
      registry->GetComponent<InventoryComponent>(world->GetPlayer());
  const ItemDatabase &itemdb = ItemDatabase::instance();

  int row = invComp.row;
  int column = invComp.column;
  ImVec2 padding = ImGui::GetStyle().FramePadding;

  ImGui::Begin("Inventory", &bIsShowingInventory,
               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

  for (int r = 0; r < row; ++r) {
    for (int c = 0; c < column; ++c) {
      int idx = r * column + c;
      ImGui::PushID(idx);

      if (c != 0) ImGui::SameLine();

      ImGui::BeginGroup();

      // Disabled Button
      if (idx >= invComp.items.size()) {
        ImGui::BeginDisabled();
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(32, 32, 32));
        ImGui::Button("", ImVec2(invNodeSize, invNodeSize));
        if (ImGui::BeginDragDropTarget()) {
          if (const ImGuiPayload *payload =
                  ImGui::AcceptDragDropPayload("DND_ITEM")) {
            IM_ASSERT(payload->DataSize == sizeof(ItemPayload));

            ItemPayload *payload_ptr =
                static_cast<ItemPayload *>(payload->Data);
            if (payload_ptr->owner != world->GetPlayer()) {
              if (registry->HasComponent<InventoryComponent>(
                      payload_ptr->owner)) {
                eventDispatcher->Publish(
                    ItemMoveEvent(payload_ptr->owner, world->GetPlayer(),
                                  payload_ptr->id, payload_ptr->amount));
              } else if (registry->HasComponent<AssemblingMachineComponent>(
                             payload_ptr->owner)) {
                eventDispatcher->Publish(AssemblyTakeOutputEvent(
                    payload_ptr->owner, world->GetPlayer(), payload_ptr->id,
                    payload_ptr->amount));
              } else {
                eventDispatcher->Publish(ItemAddEvent(
                    world->GetPlayer(), payload_ptr->id, payload_ptr->amount));
              }
            }
          }
          ImGui::EndDragDropTarget();
        }
        ImGui::PopStyleColor(1);
        ImGui::EndDisabled();
        ImGui::EndGroup();
        ImGui::PopID();
        continue;
      }

      auto &[invItemId, invItemAmt] = invComp.items[idx];

      // item amount on top of inventory item
      ImVec2 start_pos = ImGui::GetCursorScreenPos();

      ItemData itemdata = itemdb.get(invItemId);

      // create inventory node with item
      ImTextureID iconTexture = (intptr_t)(assetManager->getTexture(
          itemdb.get(invItemId).icon.c_str()));
      ImVec2 iconSize{invNodeSize - padding.x * 2.f,
                      invNodeSize - padding.y * 2.f};
      ImGui::ImageButton((const char *)itemdata.name.c_str(), iconTexture,
                         iconSize, ImVec2{uB0, vB0}, ImVec2{uB1, vB1});
      ImGui::SetItemTooltip("%s", (const char *)itemdata.description.c_str());

      // Handle dragging start
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        payload = ItemPayload{idx, world->GetPlayer(), invItemId, invItemAmt};
        ImGui::SetDragDropPayload("DND_ITEM", &payload, sizeof(ItemPayload));
        ImGui::Image(iconTexture, iconSize, ImVec2{uB0, vB0}, ImVec2{uB1, vB1});

        ImGui::EndDragDropSource();
      }

      // Handle dropping inside inventory
      if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload =
                ImGui::AcceptDragDropPayload("DND_ITEM")) {
          IM_ASSERT(payload->DataSize == sizeof(ItemPayload));

          ItemPayload *payload_ptr = static_cast<ItemPayload *>(payload->Data);

          if (payload_ptr->owner != world->GetPlayer()) {
            if (registry->HasComponent<InventoryComponent>(
                    payload_ptr->owner)) {
              eventDispatcher->Publish(
                  ItemMoveEvent(payload_ptr->owner, world->GetPlayer(),
                                payload_ptr->id, payload_ptr->amount));
            } else if (registry->HasComponent<AssemblingMachineComponent>(
                           payload_ptr->owner)) {
              eventDispatcher->Publish(AssemblyTakeOutputEvent(
                  payload_ptr->owner, world->GetPlayer(), payload_ptr->id,
                  payload_ptr->amount));
            } else {
              eventDispatcher->Publish(ItemAddEvent(
                  world->GetPlayer(), payload_ptr->id, payload_ptr->amount));
            }
          } else {
            if (payload_ptr->itemIdx < invComp.items.size()) {
              std::swap(invComp.items[idx],
                        invComp.items[payload_ptr->itemIdx]);
            }
          }
        }
        ImGui::EndDragDropTarget();
      }
      ImVec2 next_pos = ImGui::GetCursorScreenPos();

#ifdef TEXTLATER

      ImVec2 btnSize = ImGui::GetItemRectSize();
      // item amount on top of inventory item
      ImGui::SetCursorScreenPos(start_pos);

      ImGui::SetNextItemAllowOverlap();
      ImGui::Text("%s", std::to_string(invComp.items[idx].second).c_str());

      ImGui::SetCursorScreenPos(start_pos);
      ImGui::SetNextItemAllowOverlap();
      ImGui::Dummy(btnSize);
#endif

      ImGui::EndGroup();
      ImGui::PopID();
    }
  }
  ImGui::End();
}

void UISystem::AssemblingMachineUI() {
  // Find all assembling machines that should show UI
  for (auto machineEntity : registry->view<AssemblingMachineComponent>()) {
    auto &assemblingComp =
        registry->GetComponent<AssemblingMachineComponent>(machineEntity);
    if (assemblingComp.bIsShowingUI) {
      if (assemblingComp.bIsShowingRecipeSelection) {
        AssemblingMachineRecipeSelection(machineEntity);
      } else {
        // Show crafting UI
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        std::string windowName =
            "Assembling Machine##" + std::to_string(machineEntity);
        bool bIsShowingUI = assemblingComp.bIsShowingUI;

        if (ImGui::Begin(windowName.c_str(), &bIsShowingUI,
                         ImGuiWindowFlags_NoCollapse)) {
          const auto &recipeData =
              RecipeDatabase::instance().get(assemblingComp.currentRecipe);

          ImGui::Text("Recipe: %s", (const char *)recipeData.name.c_str());
          ImGui::Text("Crafting Time: %.1fs", recipeData.craftingTime);

          if (ImGui::Button("Change Recipe")) {
            assemblingComp.bIsShowingRecipeSelection = true;
          }

          ImGui::Separator();

          // Input slots
          ImGui::Text("Input:");
          const auto &itemDB = ItemDatabase::instance();

          for (size_t i = 0; i < recipeData.ingredients.size(); ++i) {
            const auto &ingredient = recipeData.ingredients[i];
            auto it = assemblingComp.inputInventory.find(ingredient.itemId);
            int currentAmount =
                (it != assemblingComp.inputInventory.end()) ? it->second : 0;

            const auto &itemData = itemDB.get(ingredient.itemId);

            if (i > 0) ImGui::SameLine();

            ImTextureID iconTexture =
                (intptr_t)(assetManager->getTexture(itemData.icon.c_str()));

            ImVec2 slotSize(64, 64);
            ImGui::BeginGroup();

            // Progress bar for required amount
            float progress =
                std::min(1.0f, (float)currentAmount / ingredient.amount);
            ImGui::ProgressBar(progress, slotSize, "");
            ImGui::SetCursorScreenPos(ImGui::GetItemRectMin());

            ImGui::Image(iconTexture, slotSize, ImVec2{uB0, vB0},
                         ImVec2{uB1, vB1});
            // Handle drag drop
            if (ImGui::BeginDragDropTarget()) {
              if (const ImGuiPayload *payload =
                      ImGui::AcceptDragDropPayload("DND_ITEM")) {
                ItemPayload *item_payload =
                    static_cast<ItemPayload *>(payload->Data);

                if (item_payload->id == ingredient.itemId) {
                  eventDispatcher->Publish(AssemblyAddInputEvent(
                      machineEntity, item_payload->owner, item_payload->id,
                      item_payload->amount));
                  ImGui::SetTooltip("Drop %s here",
                                    (const char *)itemData.name.c_str());
                }
              }
              ImGui::EndDragDropTarget();
            }

            ImGui::Text("%d/%d", currentAmount, ingredient.amount);

            ImGui::EndGroup();
          }

          ImGui::Separator();

          // Output slot
          ImGui::Text("Output:");
          const auto &outputData = itemDB.get(recipeData.outputItem);
          auto outputIt =
              assemblingComp.outputInventory.find(recipeData.outputItem);
          int outputAmount = (outputIt != assemblingComp.outputInventory.end())
                                 ? outputIt->second
                                 : 0;

          ImTextureID outputTexture =
              (intptr_t)(assetManager->getTexture(outputData.icon.c_str()));

          ImVec2 outputSlotSize(64, 64);
          ImGui::ImageButton("output", outputTexture, outputSlotSize,
                             ImVec2{uB0, vB0}, ImVec2{uB1, vB1});
          // Handle drag from output
          if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            ItemPayload outputPayload{-1, machineEntity, recipeData.outputItem,
                                      outputAmount};
            ImGui::SetDragDropPayload("DND_ITEM", &outputPayload,
                                      sizeof(ItemPayload));
            ImGui::Image(outputTexture, outputSlotSize, ImVec2{uB0, vB0},
                         ImVec2{uB1, vB1});
            ImGui::EndDragDropSource();
          }
          ImGui::Text("%d", outputAmount);

          // Show crafting progress
          if (assemblingComp.state == AssemblingMachineState::Crafting) {
            // Would need to get progress from timer system
            ImGui::Separator();
            ImGui::Text("Crafting... (Animation: %s)",
                        assemblingComp.bIsAnimating ? "ON" : "OFF");
          } else if (assemblingComp.state ==
                     AssemblingMachineState::WaitingForIngredients) {
            ImGui::Text("Waiting for ingredients");
          } else if (assemblingComp.state ==
                     AssemblingMachineState::OutputFull) {
            ImGui::Text("Output full!");
          }
        }
        ImGui::End();

        assemblingComp.bIsShowingUI = bIsShowingUI;
      }
    }
  }
}

void UISystem::AssemblingMachineRecipeSelection(EntityID entity) {
  auto &assemblingComp =
      registry->GetComponent<AssemblingMachineComponent>(entity);

  std::string windowName = "Select Recipe##" + std::to_string(entity);
  bool showSelection = assemblingComp.bIsShowingRecipeSelection;

  if (ImGui::Begin(
          windowName.c_str(), &showSelection,
          ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("Choose a recipe for the assembling machine:");
    ImGui::Separator();

    const auto &recipeDB = RecipeDatabase::instance();
    auto allRecipes = recipeDB.getAllRecipes();

    for (RecipeID recipeId : allRecipes) {
      const auto &recipeData = recipeDB.get(recipeId);

      if (ImGui::Button((const char *)recipeData.name.c_str(),
                        ImVec2(200, 0))) {
        // Set the recipe using AssemblingMachineSystem
        // For now, set it directly - should use system method
        assemblingComp.currentRecipe = recipeId;
        assemblingComp.bIsShowingRecipeSelection = false;
        assemblingComp.state = AssemblingMachineState::Idle;
        showSelection = false;
      }

      ImGui::SameLine();
      ImGui::Text("- %s (%.1fs)", (const char *)recipeData.description.c_str(),
                  recipeData.craftingTime);
    }

    if (ImGui::Button("Cancel")) {
      showSelection = false;
    }
  }
  ImGui::End();

  assemblingComp.bIsShowingRecipeSelection = showSelection;
  if (!showSelection && assemblingComp.currentRecipe == RecipeID::None) {
    assemblingComp.bIsShowingUI =
        false;  // Close UI if no recipe selected and cancelled
  }
}

void UISystem::MiningDrillUI() {
  const ItemDatabase &itemdb = ItemDatabase::instance();
  ImVec2 outputSlotSize(60, 60);
  for (auto drillEntity : registry->view<MiningDrillComponent>()) {
    auto &drillComp = registry->GetComponent<MiningDrillComponent>(drillEntity);
    if (drillComp.bIsShowingUI) {
      std::string windowName = "Mining Drill##" + std::to_string(drillEntity);
      bool bIsShowingUI = drillComp.bIsShowingUI;
      if (ImGui::Begin(windowName.c_str(), &bIsShowingUI,
                       ImGuiWindowFlags_AlwaysAutoResize |
                           ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Mining Drill");

        MiningDrillComponent &drillcomp =
            registry->GetComponent<MiningDrillComponent>(drillEntity);

        InventoryComponent &invcomp =
            registry->GetComponent<InventoryComponent>(drillEntity);

        if (invcomp.items.size() == 0) {
          ImGui::BeginDisabled();
          ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(32, 32, 32));
          ImGui::Button("output", ImVec2(invNodeSize, invNodeSize));
          ImGui::PopStyleColor(1);
          ImGui::EndDisabled();
        } else {
          ImVec2 start_pos = ImGui::GetCursorScreenPos();

          const ItemData &invdata = itemdb.get(invcomp.items[0].first);
          ImTextureID outputTexture =
              (intptr_t)(assetManager->getTexture(invdata.icon.c_str()));

          ImGui::ImageButton("output", outputTexture, outputSlotSize,
                             ImVec2{uB0, vB0}, ImVec2{uB1, vB1});
          if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            ItemPayload outputPayload{0, drillEntity, invcomp.items[0].first,
                                      invcomp.items[0].second};
            ImGui::SetDragDropPayload("DND_ITEM", &outputPayload,
                                      sizeof(ItemPayload));
            ImGui::EndDragDropSource();
          }
          ImGui::SetNextItemAllowOverlap();
          ImGui::SetCursorScreenPos(start_pos);
          ImGui::Text("%s", std::to_string(invcomp.items[0].second).c_str());
        }
      }
      ImGui::End();
      drillComp.bIsShowingUI = bIsShowingUI;
    }
  }
}

UISystem::~UISystem() = default;