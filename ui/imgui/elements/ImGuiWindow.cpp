//
// Created by petr on 10/31/20.
//

#include "ImGuiWindow.h"
#include <algorithm>
#include <utility>
#include <imgui.h>
namespace pf::ui {

ImGuiWindow::ImGuiWindow(const std::string &elementName, std::string title)
    : ImGuiContainer(elementName), title(std::move(title)) {}

void ImGuiWindow::render() {
  ImGui::Begin(title.c_str());
  std::ranges::for_each(getChildren(), [&](auto &child) { child.second->render(); });

  ImGui::End();
}
const std::string &ImGuiWindow::getTitle() const { return title; }
void ImGuiWindow::setTitle(const std::string &tit) { title = tit; }
}// namespace pf::ui