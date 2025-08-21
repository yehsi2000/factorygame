#include "System/UISystem.h"

#include <algorithm>
#include <string>

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
#include "Common.h"

constexpr float ICONSPRITE_WIDTHF = static_cast<float>(ICONSPRITE_WIDTH);
constexpr float ICONSPRITE_HEIGHTF = static_cast<float>(ICONSPRITE_HEIGHT);

constexpr float uB0 = 0 / ICONSPRITE_WIDTHF;
constexpr float vB0 = 0.f;
constexpr float uB1 = ICONSIZE_BIG / ICONSPRITE_WIDTHF;
constexpr float vB1 = ICONSIZE_BIG / ICONSPRITE_HEIGHTF;

constexpr float uM0 = ICONSIZE_BIG / ICONSPRITE_WIDTHF;
constexpr float vM0 = 0.f;
constexpr float uM1 = (ICONSIZE_BIG + ICONSIZE_MID) / ICONSPRITE_WIDTHF;
constexpr float vM1 = ICONSIZE_MID / ICONSPRITE_HEIGHTF;

constexpr float uS0 = (ICONSIZE_BIG + ICONSIZE_MID) / ICONSPRITE_WIDTHF;
constexpr float vS0 = 0.f;
constexpr float uS1 = (ICONSIZE_BIG + ICONSIZE_MID + ICONSIZE_SMALL) / ICONSPRITE_WIDTHF;
constexpr float vS1 = ICONSIZE_SMALL / ICONSPRITE_HEIGHTF;

constexpr float uT0 = (ICONSIZE_BIG + ICONSIZE_MID + ICONSIZE_SMALL) / ICONSPRITE_WIDTHF;
constexpr float vT0 = 0.f;
constexpr float uT1 = (ICONSIZE_BIG + ICONSIZE_MID + ICONSIZE_SMALL + ICONSIZE_TINY) / ICONSPRITE_WIDTHF;
constexpr float vT1 = ICONSIZE_TINY / ICONSPRITE_HEIGHTF;

constexpr float invNodeSize = 64.f;

UISystem::UISystem(GEngine *engine)
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
  }

  ImGui::Render();
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(),
                                        engine->GetRenderer());
}

void UISystem::Inventory() {
  Registry *registry = engine->GetRegistry();
  SDL_Renderer *renderer = engine->GetRenderer();
  InventoryComponent &invcomp =
      registry->GetComponent<InventoryComponent>(engine->GetPlayer());
  const ItemDatabase &itemdb = ItemDatabase::instance();

  int row = invcomp.row;
  int column = invcomp.column;
  ImVec2 padding = ImGui::GetStyle().FramePadding;

  ImGui::Begin("Inventory", &showInventory,
               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

  for (int r = 0; r < row; ++r) {
    for (int c = 0; c < column; ++c) {
      int idx = r * column + c;
      ImGui::PushID(idx);

      if (c != 0) ImGui::SameLine();

      // Disabled Button
      if (idx >= invcomp.items.size()) {
        ImGui::BeginDisabled();
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(32, 32, 32));
        ImGui::Button("", ImVec2(invNodeSize, invNodeSize));
        ImGui::PopStyleColor(1);
        ImGui::EndDisabled();
        ImGui::PopID();
        continue;
      }

      auto &[invItemId, invItemAmt] = invcomp.items[idx];

      // item amount on top of inventory item
      ImVec2 text_pos = ImGui::GetCursorScreenPos();
      ImGui::Text("%s", std::to_string(invItemAmt).c_str());
      ImGui::SetNextItemAllowOverlap();
      ImGui::SetCursorScreenPos(text_pos);

      ItemData itemdata = itemdb.get(invItemId);

      // create inventory node with item
      ImTextureID iconTexture = (intptr_t)(AssetManager::Instance().getTexture(
          itemdb.get(invItemId).icon.c_str(), renderer));
      ImVec2 iconSize =
          ImVec2(invNodeSize - padding.x * 2.f, invNodeSize - padding.y * 2.f);

      ImGui::ImageButton(itemdata.name.c_str(), iconTexture, iconSize,
                         ImVec2{uB0, vB0}, ImVec2{uB1, vB1});
      ImGui::SetItemTooltip("%s", itemdata.description.c_str());

      // Handle dragging start
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        payload = ItemPayload{idx, invItemId, invItemAmt};
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

          // swap items
          if (payload_ptr->itemIdx < invcomp.items.size()) {
            std::swap(invcomp.items[idx], invcomp.items[payload_ptr->itemIdx]);
            *payload_ptr = {0, ItemID::None, 0};
          }
        }
        ImGui::EndDragDropTarget();
      }

      ImGui::PopID();
    }
  }
  ImGui::End();
}

void UISystem::AssemblingMachineUI() {
  Registry* registry = engine->GetRegistry();
  SDL_Renderer* renderer = engine->GetRenderer();
  
  // Find all assembling machines that should show UI
  auto view = registry->view<AssemblingMachineComponent>();
  for (auto entity : view) {
    auto& machine = registry->GetComponent<AssemblingMachineComponent>(entity);
    
    if (machine.showUI) {
      if (machine.showRecipeSelection) {
        AssemblingMachineRecipeSelection(entity);
      } else {
        // Show crafting UI
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        std::string windowName = "Assembling Machine##" + std::to_string(entity);
        bool showUI = machine.showUI;
        
        if (ImGui::Begin(windowName.c_str(), &showUI, ImGuiWindowFlags_NoCollapse)) {
          const auto& recipeData = RecipeDatabase::instance().get(machine.currentRecipe);
          
          ImGui::Text("Recipe: %s", recipeData.name.c_str());
          ImGui::Text("Crafting Time: %.1fs", recipeData.craftingTime);
          
          if (ImGui::Button("Change Recipe")) {
            machine.showRecipeSelection = true;
          }
          
          ImGui::Separator();
          
          // Input slots
          ImGui::Text("Input:");
          const auto& itemDB = ItemDatabase::instance();
          
          for (size_t i = 0; i < recipeData.ingredients.size(); ++i) {
            const auto& ingredient = recipeData.ingredients[i];
            auto it = machine.inputInventory.find(ingredient.itemId);
            int currentAmount = (it != machine.inputInventory.end()) ? it->second : 0;
            
            const auto& itemData = itemDB.get(ingredient.itemId);
            
            if (i > 0) ImGui::SameLine();
            
            ImTextureID iconTexture = (intptr_t)(AssetManager::Instance().getTexture(
                itemData.icon.c_str(), renderer));
            
            ImVec2 slotSize(64, 64);
            ImGui::BeginGroup();
            
            // Progress bar for required amount
            float progress = std::min(1.0f, (float)currentAmount / ingredient.amount);
            ImGui::ProgressBar(progress, slotSize, "");
            ImGui::SetCursorScreenPos(ImGui::GetItemRectMin());
            
            ImGui::Image(iconTexture, slotSize, ImVec2{uB0, vB0}, ImVec2{uB1, vB1});
            ImGui::Text("%d/%d", currentAmount, ingredient.amount);
            
            // Handle drag drop
            if (ImGui::BeginDragDropTarget()) {
              if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ITEM")) {
                ItemPayload* item_payload = static_cast<ItemPayload*>(payload->Data);
                if (item_payload->id == ingredient.itemId) {
                  // Try to add items to assembling machine
                  // This would need to be implemented in AssemblingMachineSystem
                  // For now, just show it accepts the drop
                  ImGui::SetTooltip("Drop %s here", itemData.name.c_str());
                }
              }
              ImGui::EndDragDropTarget();
            }
            
            ImGui::EndGroup();
          }
          
          ImGui::Separator();
          
          // Output slot
          ImGui::Text("Output:");
          const auto& outputData = itemDB.get(recipeData.outputItem);
          auto outputIt = machine.outputInventory.find(recipeData.outputItem);
          int outputAmount = (outputIt != machine.outputInventory.end()) ? outputIt->second : 0;
          
          ImTextureID outputTexture = (intptr_t)(AssetManager::Instance().getTexture(
              outputData.icon.c_str(), renderer));
          
          ImVec2 outputSlotSize(64, 64);
          ImGui::ImageButton("output", outputTexture, outputSlotSize, 
                            ImVec2{uB0, vB0}, ImVec2{uB1, vB1});
          // Handle drag from output
          if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            ItemPayload outputPayload{-1, recipeData.outputItem, outputAmount};
            ImGui::SetDragDropPayload("DND_ITEM", &outputPayload, sizeof(ItemPayload));
            ImGui::Image(outputTexture, outputSlotSize, ImVec2{uB0, vB0}, ImVec2{uB1, vB1});
            ImGui::EndDragDropSource();
          }
          ImGui::Text("%d", outputAmount);
          
          // Show crafting progress
          if (machine.state == AssemblingMachineState::Crafting) {
            // Would need to get progress from timer system
            ImGui::Separator();
            ImGui::Text("Crafting... (Animation: %s)", machine.isAnimating ? "ON" : "OFF");
          } else if (machine.state == AssemblingMachineState::WaitingForIngredients) {
            ImGui::Text("Waiting for ingredients");
          } else if (machine.state == AssemblingMachineState::OutputFull) {
            ImGui::Text("Output full!");
          }
        }
        ImGui::End();
        
        machine.showUI = showUI;
      }
    }
  }
}

void UISystem::AssemblingMachineRecipeSelection(EntityID entity) {
  Registry* registry = engine->GetRegistry();
  auto& machine = registry->GetComponent<AssemblingMachineComponent>(entity);
  
  std::string windowName = "Select Recipe##" + std::to_string(entity);
  bool showSelection = machine.showRecipeSelection;
  
  if (ImGui::Begin(windowName.c_str(), &showSelection, 
                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
    
    ImGui::Text("Choose a recipe for the assembling machine:");
    ImGui::Separator();
    
    const auto& recipeDB = RecipeDatabase::instance();
    auto allRecipes = recipeDB.getAllRecipes();
    
    for (RecipeID recipeId : allRecipes) {
      const auto& recipeData = recipeDB.get(recipeId);
      
      if (ImGui::Button(recipeData.name.c_str(), ImVec2(200, 0))) {
        // Set the recipe using AssemblingMachineSystem
        // For now, set it directly - should use system method
        machine.currentRecipe = recipeId;
        machine.showRecipeSelection = false;
        machine.state = AssemblingMachineState::Idle;
        showSelection = false;
      }
      
      ImGui::SameLine();
      ImGui::Text("- %s (%.1fs)", recipeData.description.c_str(), recipeData.craftingTime);
    }
    
    if (ImGui::Button("Cancel")) {
      showSelection = false;
    }
  }
  ImGui::End();
  
  machine.showRecipeSelection = showSelection;
  if (!showSelection && machine.currentRecipe == RecipeID::None) {
    machine.showUI = false; // Close UI if no recipe selected and cancelled
  }
}