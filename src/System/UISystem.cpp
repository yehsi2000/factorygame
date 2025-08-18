#include "System/UISystem.h"

#include <algorithm>
#include <string>

#include "Components/InventoryComponent.h"
#include "Core/GEngine.h"
#include "Core/Item.h"
#include "Core/Registry.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

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
  InventoryComponent &invcomp =
      registry->GetComponent<InventoryComponent>(engine->GetPlayer());
  int row = invcomp.row;
  int column = invcomp.column;

  ImGui::Begin("Inventory", &showInventory,
               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
  for (int r = 0; r < row; ++r) {
    for (int c = 0; c < column; ++c) {
      int idx = r * column + c;
      ImGui::PushID(idx);

      if (c != 0) ImGui::SameLine();

      if (idx >= invcomp.items.size()) {
        ImGui::BeginDisabled();
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(60, 60, 60));
        ImGui::Button("", ImVec2(60, 60));
        ImGui::PopStyleColor(1);
        ImGui::EndDisabled();
        ImGui::PopID();
        continue;
      }
      ImVec2 text_pos = ImGui::GetCursorScreenPos();

      ImGui::Text("%s", std::to_string(invcomp.items[idx].second).c_str());

      ImGui::SetNextItemAllowOverlap();
      ImGui::SetCursorScreenPos(text_pos);
      ImGui::Button((const char *)ItemDatabase::instance()
                        .get(invcomp.items[idx].first)
                        .name.c_str(),
                    ImVec2(60, 60));
      ImGui::SetItemTooltip("%s", (const char *)ItemDatabase::instance()
                                      .get(invcomp.items[idx].first)
                                      .description.c_str());
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        payload = ItemPayload{idx, invcomp.items[idx].first,
                              invcomp.items[idx].second};
        ImGui::SetDragDropPayload("DND_ITEM", &payload, sizeof(ItemPayload));
        ImGui::Text("Dragging %s", "iteminfo");
        ImGui::EndDragDropSource();
      }
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