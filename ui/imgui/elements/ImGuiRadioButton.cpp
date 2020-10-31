//
// Created by petr on 10/31/20.
//

#include "ImGuiRadioButton.h"
namespace pf::ui {

ImGuiRadioButton::ImGuiRadioButton(const std::string &elementName, const std::string &caption,
                                   bool value)
    : ImGuiElement(elementName), ImGuiCaptionedElement(elementName, caption),
      ImGuiValueObservableElement(elementName, value) {}

void ImGuiRadioButton::render() {
  const auto oldValue = getValue();
  if (ImGui::RadioButton(getCaption().c_str(), getValue())) {
    setValue(true);
  }
  if (oldValue != getValue() && getValue()) { notifyValueChanged(); }
}
bool ImGuiRadioButton::isSelected() const {
  return getValue();
}

}// namespace pf::ui