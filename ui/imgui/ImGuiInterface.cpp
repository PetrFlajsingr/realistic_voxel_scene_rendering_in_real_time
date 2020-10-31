//
// Created by petr on 10/27/20.
//

#include "ImGuiInterface.h"

namespace pf::ui {

ImGuiInterface::ImGuiInterface(ImGuiConfigFlags flags) : ImGuiContainer("Main"), io(baseInit(flags)) {
}
ImGuiIO &ImGuiInterface::baseInit(ImGuiConfigFlags flags) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  auto& imguiIo = ImGui::GetIO();
  imguiIo.ConfigFlags |= flags;
  ImGui::StyleColorsDark();
  return imguiIo;
}

ImGuiIO &ImGuiInterface::getIo() const { return io; }

}