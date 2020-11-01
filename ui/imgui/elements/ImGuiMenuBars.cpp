//
// Created by petr on 11/1/20.
//

#include "ImGuiMenuBars.h"
#include "../utils/Visitor.h"
#include <algorithm>
#include <imgui.h>

namespace pf::ui {

ImGuiMenuItem::ImGuiMenuItem(const std::string &elementName, const std::string &caption)
    : ImGuiElement(elementName), ImGuiCaptionedElement(elementName, caption) {}

void ImGuiMenuItem::renderImpl() {
  if (ImGui::MenuItem(getCaption().c_str())) { onClick(); }
}

void ImGuiSubMenu::renderImpl() {
  if (ImGui::BeginMenu(getCaption().c_str())) {
    renderItems();
    ImGui::EndMenu();
  }
}

ImGuiSubMenu::ImGuiSubMenu(const std::string &elementName, const std::string &caption)
    : ImGuiElement(elementName), ImGuiCaptionedElement(elementName, caption) {}

ImGuiWindowMenuBar::ImGuiWindowMenuBar(const std::string &elementName)
    : ImGuiElement(elementName) {}

void ImGuiWindowMenuBar::renderImpl() {
  ImGui::BeginMenuBar();
  renderItems();
  ImGui::EndMenuBar();
}
ImGuiAppMenuBar::ImGuiAppMenuBar(const std::string &elementName) : ImGuiElement(elementName) {}

void ImGuiAppMenuBar::renderImpl() {
  ImGui::BeginMainMenuBar();
  renderItems();
  ImGui::EndMainMenuBar();
}

ImGuiSubMenu &ImGuiMenuContainer::addSubmenu(const std::string &name, const std::string &caption) {
  items.emplace_back(ImGuiSubMenu(name, caption));
  return std::get<ImGuiSubMenu>(items.back());
}

ImGuiMenuItem &ImGuiMenuContainer::addItem(const std::string &name, const std::string &caption) {
  items.emplace_back(ImGuiMenuItem(name, caption));
  return std::get<ImGuiMenuItem>(items.back());
}
void ImGuiMenuContainer::removeItem(const std::string &name) {
  if (const auto iter = std::ranges::find_if(
          items,
          [name](auto &item) {
            return std::visit(
                Visitor{[name](ImGuiSubMenu &item) { return item.getName() == name; },
                        [name](ImGuiMenuItem &item) { return item.getName() == name; }},
                item);
          });
      iter != items.end()) {
    items.erase(iter);
  }
}
void ImGuiMenuContainer::renderItems() {
  std::ranges::for_each(items, [](auto &item) {
    std::visit(Visitor{[](ImGuiMenuItem &item) { item.render(); },
                       [](ImGuiSubMenu &subMenu) { subMenu.render(); }},
               item);
  });
}
}// namespace pf::ui
