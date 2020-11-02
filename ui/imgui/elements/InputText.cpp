//
// Created by petr on 10/31/20.
//

#include "InputText.h"
#include <imgui.h>

#include <utility>

namespace pf::ui::ig {
InputText::InputText(const std::string &elementName, std::string caption,
                               const std::string &text, TextInputType textInputType)
    : Element(elementName), Text(elementName, text),
      LabeledElement(elementName, std::move(caption)), ValueObservableElement(elementName, ""), inputType(textInputType) {
  setValue(text);
}

void InputText::renderImpl() {
  if (inputType == TextInputType::SingleLine) {
    ImGui::InputText(getLabel().c_str(), buffer, 256);
  } else {
    ImGui::InputTextMultiline(getLabel().c_str(), buffer, 256);
  }
  if (strcmp(buffer, getText().c_str()) != 0) {
    setText(buffer);
    setValue(getText());
    notifyValueChanged();
  }
}
void InputText::clear() {
  setText("");
  buffer[0] = '\0';
}

}// namespace pf::ui