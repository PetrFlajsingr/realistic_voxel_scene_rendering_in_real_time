//
// Created by petr on 10/27/20.
//

#include "ImGuiBase.h"

namespace pf::ui {

ImGuiBase::ImGuiBase(ImGuiConfigFlags flags) : io(baseInit(flags)) {
}
ImGuiIO &ImGuiBase::baseInit(ImGuiConfigFlags flags) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  auto& imguiIo = ImGui::GetIO();
  imguiIo.ConfigFlags |= flags;
  ImGui::StyleColorsDark();
  return imguiIo;
}

ImGuiIO &ImGuiBase::getIo() const { return io; }

}