//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIPANEL_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIPANEL_H

#include "interface/ImGuiContainer.h"
#include <imgui.h>

namespace pf::ui {
class ImGuiPanel : public ImGuiContainer {
 public:
  ImGuiPanel(const std::string &elementName, std::string title, const ImVec2 &panelSize = {0, 0});

 protected:
  void renderImpl() override;

 private:
  std::string title;
  ImVec2 size;
};

}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIPANEL_H
