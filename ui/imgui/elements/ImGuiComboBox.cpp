//
// Created by petr on 10/31/20.
//

#include "ImGuiComboBox.h"
#include <algorithm>
#include <imgui.h>
#include <range/v3/view.hpp>
#include <utility>

namespace pf::ui {

ImGuiComboBox::ImGuiComboBox(const std::string &elementName, const std::string &caption,
                             std::string previewValue, std::vector<std::string> items)
    : ImGuiElement(elementName),
      ImGuiCaptionedElement(elementName, caption), ImGuiValueObservableElement<std::string_view>(
                                                       elementName, ""),
      items(std::move(items)), previewValue(std::move(previewValue)) {}

void ImGuiComboBox::render() {
  using namespace ranges::views;
  if (ImGui::BeginCombo(getCaption().c_str(),
                        selectedItemIndex.has_value() ? items[*selectedItemIndex].c_str()
                                                      : previewValue.c_str())) {
    auto cStrItems = items | transform([](const auto &str) { return str.c_str(); });
    std::ranges::for_each(cStrItems | enumerate, [&](auto idxPtr) {
      const auto [idx, ptr] = idxPtr;
      auto isSelected = selectedItemIndex.has_value() && *selectedItemIndex == idx;
      ImGui::Selectable(ptr, &isSelected);
      if (isSelected) {
        if (selectedItemIndex.has_value() && *selectedItemIndex != idx) {
          setValue(items[*selectedItemIndex]);
          notifyValueChanged();
        }
        selectedItemIndex = idx;
      }
    });
    ImGui::EndCombo();
  }
}
std::optional<std::string_view> ImGuiComboBox::getSelectedItem() {
  if (!selectedItemIndex.has_value()) { return std::nullopt; }
  return items[*selectedItemIndex];
}
void ImGuiComboBox::removeItem(const std::string &item) {
  if (const auto iter = std::ranges::find(items, item); iter != items.end()) {
    items.erase(iter);
    selectedItemIndex = std::nullopt;
  }
}
void ImGuiComboBox::addItem(const std::string &item) { items.emplace_back(item); }

}// namespace pf::ui