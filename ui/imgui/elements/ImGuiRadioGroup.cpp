//
// Created by petr on 10/31/20.
//

#include "ImGuiRadioGroup.h"
#include <range/v3/view.hpp>
#include <utility>

namespace pf::ui {

ImGuiRadioGroup::ImGuiRadioGroup(const std::string &elementName, const std::string &caption,
                                 std::vector<ImGuiRadioButton> buttons,
                                 const std::optional<std::size_t> &selectedButtonIndex)
    : ImGuiElement(elementName),
      ImGuiCaptionedElement(elementName, caption), ImGuiValueObservableElement<std::string_view>(
                                                       elementName, ""),
      buttons(std::move(buttons)), selectedButtonIndex(selectedButtonIndex) {}

void ImGuiRadioGroup::renderImpl() {
  ImGui::Separator();
  ImGui::Text("%s:", getCaption().c_str());
  std::ranges::for_each(buttons, [](auto &button) { button.renderImpl(); });
  std::optional<std::size_t> newSelection = std::nullopt;
  std::ranges::for_each(buttons | ranges::views::enumerate, [&](auto idxBtn) {
    auto &[idx, btn] = idxBtn;
    if (btn.isSelected()) {
      if (!selectedButtonIndex.has_value() || idx != *selectedButtonIndex) { newSelection = idx; }
    }
  });
  if (newSelection.has_value()) {
    auto &selectedButton = buttons[*newSelection];
    std::ranges::for_each(buttons, [&](auto &button) {
      if (&button != &selectedButton) { button.setValue(false); }
    });
    selectedButton.notifyValueChanged();
    setValue(selectedButton.getCaption());
    selectedButtonIndex = newSelection;
    notifyValueChanged();
  }
  ImGui::Separator();
}

void ImGuiRadioGroup::addButton(const std::string &elementName, const std::string &caption,
                                bool value) {
  buttons.emplace_back(elementName, caption, value);
}
}// namespace pf::ui