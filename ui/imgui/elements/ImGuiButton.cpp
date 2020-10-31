//
// Created by petr on 10/31/20.
//

#include "ImGuiButton.h"
#include <utility>

namespace pf::ui {

ImGuiButton::ImGuiButton(const std::string &name, std::string caption, const ImVec2 &size)
    : ImGuiElement(name), ImGuiCaptionedElement(name, std::move(caption)),
      ImGuiResizableElement(name, size) {}

void ImGuiButton::render() {
  if (ImGui::Button(getCaption().c_str(), getSize())) { onClick(); }
}
}// namespace pf::ui