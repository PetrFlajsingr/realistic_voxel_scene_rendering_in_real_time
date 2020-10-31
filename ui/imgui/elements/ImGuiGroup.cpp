//
// Created by petr on 10/31/20.
//

#include "ImGuiGroup.h"
#include <imgui.h>

namespace pf::ui {

ImGuiGroup::ImGuiGroup(const std::string &elementName, const std::string &caption)
    : ImGuiElement(elementName), ImGuiContainer(elementName),
      ImGuiCaptionedElement(elementName, caption) {}

void ImGuiGroup::renderImpl() {
  ImGui::BeginGroup();
  ImGui::Text("%s:", getCaption().c_str());
  ImGui::Separator();
  std::ranges::for_each(getChildren(), [](auto &child) { child.get().render(); });
  ImGui::Separator();
  ImGui::EndGroup();
}

}// namespace pf::ui