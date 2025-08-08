#include "System/UISystem.h"

#include <algorithm>
#include <format>

#include "Components/InventoryComponent.h"
#include "Core/Entity.h"
#include "Core/GEngine.h"
#include "Core/Registry.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

UISystem::UISystem(GEngine *engine) : engine(engine) {}

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

      char str[5] = {};
      std::to_chars(str, str + 5, invcomp.items[idx].second);
      ImGui::Text("%s", str);

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
        ImGui::SetDragDropPayload("DND_ITEM", &idx, sizeof(int));
        ImGui::Text("Dragging %s", "iteminfo");
        ImGui::EndDragDropSource();
      }

      if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload =
                ImGui::AcceptDragDropPayload("DND_ITEM")) {
          IM_ASSERT(payload->DataSize == sizeof(int));
          int payload_n = *(const int *)payload->Data;
          // swap items
          if (payload_n < invcomp.items.size())
            std::swap(invcomp.items[idx], invcomp.items[payload_n]);
        }
        ImGui::EndDragDropTarget();
      }

      ImGui::PopID();
    }
  }
  ImGui::End();
}