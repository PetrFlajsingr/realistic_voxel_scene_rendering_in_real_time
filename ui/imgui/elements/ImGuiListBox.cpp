//
// Created by petr on 11/1/20.
//

#include "ImGuiListBox.h"
#include <imgui.h>
#include <range/v3/view.hpp>
#include <utility>

namespace pf::ui {

ImGuiListBox::ImGuiListBox(const std::string &elementName, const std::string &caption,
                           std::vector<std::string> items_, int selectedIdx, int heightInItems)
    : ImGuiElement(elementName), ImGuiCaptionedElement(elementName, caption),
      ImGuiValueObservableElement(elementName, ""), items(std::move(items_)),
      currentItemIdx(selectedIdx), height(heightInItems) {}

void ImGuiListBox::renderImpl() {
  const auto cStrItems = items
      | ranges::views::transform([](const auto &str) { return str.c_str(); }) | ranges::to_vector;
  const auto oldIdx = currentItemIdx;
  ImGui::ListBox(getCaption().c_str(), &currentItemIdx, cStrItems.data(), cStrItems.size(), height);
  if (currentItemIdx != oldIdx) {
    setValue(items[currentItemIdx]);
    notifyValueChanged();
  }
}
void ImGuiListBox::addItem(std::string item) {
  items.emplace_back(std::move(item));
}
std::string_view ImGuiListBox::getSelectedItem() const {
  return items[currentItemIdx];
}

}// namespace pf::ui