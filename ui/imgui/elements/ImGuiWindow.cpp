//
// Created by petr on 10/31/20.
//

#include "ImGuiWindow.h"
#include <algorithm>
#include <imgui.h>
#include <utility>
namespace pf::ui {

ImGuiWindow::ImGuiWindow(const std::string &elementName, std::string title)
    : ImGuiElement(elementName), ImGuiContainer(elementName), title(std::move(title)) {}

void ImGuiWindow::renderImpl() {
  ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar);
  if (hasMenuBar()) { menuBar->render(); }
  std::ranges::for_each(getChildren(), [&](auto &child) { child.get().render(); });
  ImGui::End();
}

const std::string &ImGuiWindow::getTitle() const { return title; }

void ImGuiWindow::setTitle(const std::string &tit) { title = tit; }

ImGuiWindowMenuBar &ImGuiWindow::getMenuBar() {
  if (!menuBar.has_value()) { menuBar = ImGuiWindowMenuBar(getName() + "_menu_bar"); }
  return *menuBar;
}

bool ImGuiWindow::hasMenuBar() const { return menuBar.has_value(); }
}// namespace pf::ui