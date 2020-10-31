//
// Created by petr on 10/31/20.
//

#include "ImGuiCheckbox.h"
#include <imgui.h>
namespace pf::ui {

ImGuiCheckbox::ImGuiCheckbox(const std::string &elementName, const std::string &caption, bool value)
    : ImGuiElement(elementName), ImGuiValueObservableElement(elementName, value),
      ImGuiCaptionedElement(elementName, caption) {}

void ImGuiCheckbox::renderImpl() {
  const auto oldValue = getValue();
  ImGui::Checkbox(getCaption().c_str(), getValueAddress());
  if (getValue() != oldValue) { notifyValueChanged(); }
}

}// namespace pf::ui