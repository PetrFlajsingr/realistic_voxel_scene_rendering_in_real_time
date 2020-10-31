//
// Created by petr on 10/31/20.
//

#include "ImGuiButton.h"
#include <utility>

namespace pf::ui {

ImGuiButton::ImGuiButton(const std::string &name, std::string caption, ButtonType buttonType,
                         const ImVec2 &size)
    : ImGuiElement(name), ImGuiCaptionedElement(name, std::move(caption)),
      ImGuiResizableElement(name, size), type(buttonType) {}

void ImGuiButton::render() {
  switch (type) {
    case ButtonType::Normal:
      if (ImGui::Button(getCaption().c_str(), getSize())) { onClick(); }
      break;
    case ButtonType::Small:
      if (ImGui::SmallButton(getCaption().c_str())) { onClick(); }
      break;
    case ButtonType::ArrowUp:
      if (ImGui::ArrowButton(getCaption().c_str(), ImGuiDir_::ImGuiDir_Up)) { onClick(); }
      break;
    case ButtonType::ArrowLeft:
      if (ImGui::ArrowButton(getCaption().c_str(), ImGuiDir_::ImGuiDir_Left)) { onClick(); }
      break;
    case ButtonType::ArrowRight:
      if (ImGui::ArrowButton(getCaption().c_str(), ImGuiDir_::ImGuiDir_Right)) { onClick(); }
      break;
    case ButtonType::ArrowDown:
      if (ImGui::ArrowButton(getCaption().c_str(), ImGuiDir_::ImGuiDir_Down)) { onClick(); }
      break;
  }
}
ButtonType ImGuiButton::getType() const { return type; }
void ImGuiButton::setType(ButtonType buttonType) { type = buttonType; }
}// namespace pf::ui