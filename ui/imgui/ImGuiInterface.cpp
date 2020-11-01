//
// Created by petr on 10/27/20.
//

#include "ImGuiInterface.h"

namespace pf::ui {

ImGuiInterface::ImGuiInterface(ImGuiConfigFlags flags)
    : ImGuiContainer("Main"), io(baseInit(flags)) {}
ImGuiIO &ImGuiInterface::baseInit(ImGuiConfigFlags flags) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  auto &imguiIo = ImGui::GetIO();
  imguiIo.ConfigFlags |= flags;
  ImGui::StyleColorsDark();
  return imguiIo;
}

ImGuiIO &ImGuiInterface::getIo() const { return io; }
std::shared_ptr<ImGuiDialog> ImGuiInterface::createDialog(const std::string &elementName,
                                                          const std::string &caption, Modal modal) {
  auto result = std::make_shared<ImGuiDialog>(*this, elementName, caption, modal);
  addChild(result);
  return result;
}

ImGuiAppMenuBar &ImGuiInterface::getMenuBar() {
  if (!menuBar.has_value()) {
    menuBar = ImGuiAppMenuBar("app_menu_bar");
  }
  return *menuBar;
}
bool ImGuiInterface::hasMenuBar() const {
  return menuBar.has_value();
}

}// namespace pf::ui