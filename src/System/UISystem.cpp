#include "System/UISystem.h"

#include <algorithm>
#include <string>

#include "Components/InventoryComponent.h"
#include "Core/AssetManager.h"
#include "Core/GEngine.h"
#include "Core/Item.h"
#include "Core/Registry.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "Common.h"

constexpr float uB0 = 0.f / ICONSPRITE_WIDTH;
constexpr float vB0 = 0.f;
constexpr float uB1 = ICONSIZE_BIG / ICONSPRITE_WIDTH;
constexpr float vB1 = ICONSIZE_BIG / ICONSPRITE_HEIGHT;

constexpr float uM0 = ICONSIZE_BIG / ICONSPRITE_WIDTH;
constexpr float vM0 = 0.f;
constexpr float uM1 = (ICONSIZE_BIG + ICONSIZE_MID) / ICONSPRITE_WIDTH;
constexpr float vM1 = ICONSIZE_MID / ICONSPRITE_HEIGHT;

constexpr float uS0 = (ICONSIZE_BIG + ICONSIZE_MID) / ICONSPRITE_WIDTH;
constexpr float vS0 = 0.f;
constexpr float uS1 = (ICONSIZE_BIG + ICONSIZE_MID + ICONSIZE_SMALL) / ICONSPRITE_WIDTH;
constexpr float vS1 = ICONSIZE_SMALL / ICONSPRITE_HEIGHT;

constexpr float uT0 = (ICONSIZE_BIG + ICONSIZE_MID + ICONSIZE_SMALL) / ICONSPRITE_WIDTH;
constexpr float vT0 = 0.f;
constexpr float uT1 = (ICONSIZE_BIG + ICONSIZE_MID + ICONSIZE_SMALL + ICONSIZE_TINY) / ICONSPRITE_WIDTH;
constexpr float vT1 = ICONSIZE_TINY / ICONSPRITE_HEIGHT;

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

      auto &[invItemId, invItemAmt] = invcomp.items[idx];

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