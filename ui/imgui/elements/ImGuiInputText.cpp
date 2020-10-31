//
// Created by petr on 10/31/20.
//

#include "ImGuiInputText.h"
#include <imgui.h>

#include <utility>

namespace pf::ui {
ImGuiInputText::ImGuiInputText(const std::string &elementName, std::string caption,
                               const std::string &text)
    : ImGuiElement(elementName), ImGuiText(elementName, text),
      ImGuiCaptionedElement(elementName, std::move(caption)),
      ImGuiValueObservableElement(elementName, "") {
  setValue(text);
}

void ImGuiInputText::render() {
  ImGui::InputText(getCaption().c_str(), buffer, 256);
  if (strcmp(buffer, getText().c_str()) != 0) {
    setText(buffer);
    setValue(getText());
    notifyValueChanged();
  }
}

}// namespace pf::ui