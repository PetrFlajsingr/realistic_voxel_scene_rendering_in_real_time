//
// Created by petr on 10/31/20.
//

#include "ImGuiInputText.h"
#include <imgui.h>

#include <utility>

namespace pf::ui {
ImGuiInputText::ImGuiInputText(const std::string &elementName, std::string caption,
                               const std::string &text, TextInputType textInputType)
    : ImGuiElement(elementName), ImGuiText(elementName, text),
      ImGuiCaptionedElement(elementName, std::move(caption)),
      ImGuiValueObservableElement(elementName, ""), inputType(textInputType) {
  setValue(text);
}

void ImGuiInputText::renderImpl() {
  if (inputType == TextInputType::SingleLine) {
    ImGui::InputText(getCaption().c_str(), buffer, 256);
  } else {
    ImGui::InputTextMultiline(getCaption().c_str(), buffer, 256);
  }
  if (strcmp(buffer, getText().c_str()) != 0) {
    setText(buffer);
    setValue(getText());
    notifyValueChanged();
  }
}
void ImGuiInputText::clear() {
  setText("");
  buffer[0] = '\0';
}

}// namespace pf::ui