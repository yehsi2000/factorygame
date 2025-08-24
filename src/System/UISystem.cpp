#include "System/UISystem.h"

#include <Components/MiningDrillComponent.h>

#include <algorithm>
#include <string>

#include "Common.h"
#include "Components/AssemblingMachineComponent.h"
#include "Components/InventoryComponent.h"
#include "Core/AssetManager.h"
#include "Core/GEngine.h"
#include "Core/Item.h"
#include "Core/Recipe.h"
#include "Core/Registry.h"
#include "System/AssemblingMachineSystem.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
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

UISystem::UISystem(GEngine* engine)
    : engine(engine),
      showInventoryEvent(
          engine->GetDispatcher()->Subscribe<ToggleInventoryEvent>(
              [this](ToggleInventoryEvent e) {
                showInventory = !showInventory;
              })) {}

void UISystem::Update() {
  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui_ImplSDL2_NewFrame();

  ImGui::NewFrame();
  {
    if (demoShow) ImGui::ShowDemoWindow(&demoShow);
    if (showInventory) Inventory();
    AssemblingMachineUI();
    MiningDrillUI();
  }

  ImGui::Render();
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(),
                                        engine->GetRenderer());
}

void UISystem::Inventory() {
  Registry* reg = engine->GetRegistry();
  SDL_Renderer* renderer = engine->GetRenderer();
  InventoryComponent& invComp =
      reg->GetComponent<InventoryComponent>(engine->GetPlayer());
  const ItemDatabase& itemdb = ItemDatabase::instance();

  int row = invComp.row;
  int column = invComp.column;
  ImVec2 padding = ImGui::GetStyle().FramePadding;

  ImGui::Begin("Inventory", &showInventory,
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
          if (const ImGuiPayload* payload =
                  ImGui::AcceptDragDropPayload("DND_ITEM")) {
            IM_ASSERT(payload->DataSize == sizeof(ItemPayload));

            ItemPayload* payload_ptr = static_cast<ItemPayload*>(payload->Data);
            if (payload_ptr->owner != engine->GetPlayer()) {
              if (reg->HasComponent<InventoryComponent>(payload_ptr->owner)) {
                engine->GetDispatcher()->Publish(
                    ItemMoveEvent(payload_ptr->owner, engine->GetPlayer(),
                                  payload_ptr->id, payload_ptr->amount));
              } else {
                engine->GetDispatcher()->Publish(ItemAddEvent(
                    engine->GetPlayer(), payload_ptr->id, payload_ptr->amount));
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

      auto& [invItemId, invItemAmt] = invComp.items[idx];

      // item amount on top of inventory item
      ImVec2 start_pos = ImGui::GetCursorScreenPos();

      ItemData itemdata = itemdb.get(invItemId);

      // create inventory node with item
      ImTextureID iconTexture = (intptr_t)(AssetManager::Instance().getTexture(
          itemdb.get(invItemId).icon.c_str(), renderer));
      ImVec2 iconSize{invNodeSize - padding.x * 2.f,
                      invNodeSize - padding.y * 2.f};
      ImGui::ImageButton((const char*)itemdata.name.c_str(), iconTexture,
                         iconSize, ImVec2{uB0, vB0}, ImVec2{uB1, vB1});
      ImGui::SetItemTooltip("%s", (const char*)itemdata.description.c_str());

      // Handle dragging start
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        payload = ItemPayload{idx, engine->GetPlayer(), invItemId, invItemAmt};
        ImGui::SetDragDropPayload("DND_ITEM", &payload, sizeof(ItemPayload));
        ImGui::Image(iconTexture, iconSize, ImVec2{uB0, vB0}, ImVec2{uB1, vB1});

        ImGui::EndDragDropSource();
      }

      // Handle dropping inside inventory
      if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("DND_ITEM")) {
          IM_ASSERT(payload->DataSize == sizeof(ItemPayload));

          ItemPayload* payload_ptr = static_cast<ItemPayload*>(payload->Data);

          if (payload_ptr->owner != engine->GetPlayer()) {
            if (reg->HasComponent<InventoryComponent>(payload_ptr->owner)) {
              engine->GetDispatcher()->Publish(
                  ItemMoveEvent(payload_ptr->owner, engine->GetPlayer(),
                                payload_ptr->id, payload_ptr->amount));
            } else if (reg->HasComponent<AssemblingMachineComponent>(
                           payload_ptr->owner)) {
              engine->GetDispatcher()->Publish(AssemblyTakeOutputEvent(
                  payload_ptr->owner, engine->GetPlayer(), payload_ptr->id,
                  payload_ptr->amount));
            } else {
              engine->GetDispatcher()->Publish(ItemAddEvent(
                  engine->GetPlayer(), payload_ptr->id, payload_ptr->amount));
            }
          } else {
            if (payload_ptr->itemIdx < invComp.items.size()) {
              std::swap(invComp.items[idx],
                        invComp.items[payload_ptr->itemIdx]);
              *payload_ptr = {0, engine->GetPlayer(), ItemID::None, 0};
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
  Registry* reg = engine->GetRegistry();
  SDL_Renderer* renderer = engine->GetRenderer();

  // Find all assembling machines that should show UI
  for (auto machineEntity : reg->view<AssemblingMachineComponent>()) {
    auto& assemblingComp = reg->GetComponent<AssemblingMachineComponent>(machineEntity);
    if (assemblingComp.showUI) {
      if (assemblingComp.showRecipeSelection) {
        AssemblingMachineRecipeSelection(machineEntity);
      } else {
        // Show crafting UI
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        std::string windowName =
            "Assembling Machine##" + std::to_string(machineEntity);
        bool showUI = assemblingComp.showUI;

        if (ImGui::Begin(windowName.c_str(), &showUI,
                         ImGuiWindowFlags_NoCollapse)) {
          const auto& recipeData =
              RecipeDatabase::instance().get(assemblingComp.currentRecipe);

          ImGui::Text("Recipe: %s", (const char*)recipeData.name.c_str());
          ImGui::Text("Crafting Time: %.1fs", recipeData.craftingTime);

          if (ImGui::Button("Change Recipe")) {
            assemblingComp.showRecipeSelection = true;
          }

          ImGui::Separator();

          // Input slots
          ImGui::Text("Input:");
          const auto& itemDB = ItemDatabase::instance();

          for (size_t i = 0; i < recipeData.ingredients.size(); ++i) {
            const auto& ingredient = recipeData.ingredients[i];
            auto it = assemblingComp.inputInventory.find(ingredient.itemId);
            int currentAmount =
                (it != assemblingComp.inputInventory.end()) ? it->second : 0;

            const auto& itemData = itemDB.get(ingredient.itemId);

            if (i > 0) ImGui::SameLine();

            ImTextureID iconTexture =
                (intptr_t)(AssetManager::Instance().getTexture(
                    itemData.icon.c_str(), renderer));

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
              if (const ImGuiPayload* payload =
                      ImGui::AcceptDragDropPayload("DND_ITEM")) {
                ItemPayload* item_payload =
                    static_cast<ItemPayload*>(payload->Data);

                if (item_payload->id == ingredient.itemId) {
                  engine->GetDispatcher()->Publish(AssemblyAddInputEvent(
                      machineEntity, item_payload->owner, item_payload->id, item_payload->amount));
                  ImGui::SetTooltip("Drop %s here",
                                    (const char*)itemData.name.c_str());
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
          const auto& outputData = itemDB.get(recipeData.outputItem);
          auto outputIt = assemblingComp.outputInventory.find(recipeData.outputItem);
          int outputAmount = (outputIt != assemblingComp.outputInventory.end())
                                 ? outputIt->second
                                 : 0;

          ImTextureID outputTexture =
              (intptr_t)(AssetManager::Instance().getTexture(
                  outputData.icon.c_str(), renderer));

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
                        assemblingComp.isAnimating ? "ON" : "OFF");
          } else if (assemblingComp.state ==
                     AssemblingMachineState::WaitingForIngredients) {
            ImGui::Text("Waiting for ingredients");
          } else if (assemblingComp.state == AssemblingMachineState::OutputFull) {
            ImGui::Text("Output full!");
          }
        }
        ImGui::End();

        assemblingComp.showUI = showUI;
      }
    }
  }
}

void UISystem::AssemblingMachineRecipeSelection(EntityID entity) {
  Registry* reg = engine->GetRegistry();
  auto& assemblingComp = reg->GetComponent<AssemblingMachineComponent>(entity);

  std::string windowName = "Select Recipe##" + std::to_string(entity);
  bool showSelection = assemblingComp.showRecipeSelection;

  if (ImGui::Begin(
          windowName.c_str(), &showSelection,
          ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("Choose a recipe for the assembling machine:");
    ImGui::Separator();

    const auto& recipeDB = RecipeDatabase::instance();
    auto allRecipes = recipeDB.getAllRecipes();

    for (RecipeID recipeId : allRecipes) {
      const auto& recipeData = recipeDB.get(recipeId);

      if (ImGui::Button((const char*)recipeData.name.c_str(), ImVec2(200, 0))) {
        // Set the recipe using AssemblingMachineSystem
        // For now, set it directly - should use system method
        assemblingComp.currentRecipe = recipeId;
        assemblingComp.showRecipeSelection = false;
        assemblingComp.state = AssemblingMachineState::Idle;
        showSelection = false;
      }

      ImGui::SameLine();
      ImGui::Text("- %s (%.1fs)", (const char*)recipeData.description.c_str(),
                  recipeData.craftingTime);
    }

    if (ImGui::Button("Cancel")) {
      showSelection = false;
    }
  }
  ImGui::End();

  assemblingComp.showRecipeSelection = showSelection;
  if (!showSelection && assemblingComp.currentRecipe == RecipeID::None) {
    assemblingComp.showUI = false;  // Close UI if no recipe selected and cancelled
  }
}

void UISystem::MiningDrillUI() {
  Registry* reg = engine->GetRegistry();
  const ItemDatabase& itemdb = ItemDatabase::instance();
  ImVec2 outputSlotSize(60, 60);
  for (auto drillEntity : reg->view<MiningDrillComponent>()) {
    auto& drillComp = reg->GetComponent<MiningDrillComponent>(drillEntity);
    if (drillComp.showUI) {
      std::string windowName = "Mining Drill##" + std::to_string(drillEntity);
      bool showUI = drillComp.showUI;
      if (ImGui::Begin(windowName.c_str(), &showUI,
                       ImGuiWindowFlags_AlwaysAutoResize |
                           ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Mining Drill");

        MiningDrillComponent& drillcomp =
            reg->GetComponent<MiningDrillComponent>(drillEntity);

        InventoryComponent& invcomp =
            reg->GetComponent<InventoryComponent>(drillEntity);

        if (invcomp.items.size() == 0) {
          ImGui::BeginDisabled();
          ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(32, 32, 32));
          ImGui::Button("output", ImVec2(invNodeSize, invNodeSize));
          ImGui::PopStyleColor(1);
          ImGui::EndDisabled();
        } else {
          ImVec2 start_pos = ImGui::GetCursorScreenPos();

          const ItemData& invdata = itemdb.get(invcomp.items[0].first);
          ImTextureID outputTexture =
              (intptr_t)(AssetManager::Instance().getTexture(
                  invdata.icon.c_str(), engine->GetRenderer()));

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
      drillComp.showUI = showUI;
    }
  }
}