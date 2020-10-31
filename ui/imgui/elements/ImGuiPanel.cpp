//
// Created by petr on 10/31/20.
//

#include "ImGuiPanel.h"
#include "imgui.h"
#include <algorithm>
#include <utility>

namespace pf::ui {

ImGuiPanel::ImGuiPanel(const std::string &elementName, std::string title, const ImVec2 &panelSize)
    : ImGuiElement(elementName), ImGuiContainer(elementName), title(std::move(title)),
      size(panelSize) {}

void ImGuiPanel::renderImpl() {
  ImGui::BeginChild(title.c_str(), size);
  std::ranges::for_each(getChildren(), [&](auto &child) { child.get().render(); });
  ImGui::EndChild();
}

}// namespace pf::ui