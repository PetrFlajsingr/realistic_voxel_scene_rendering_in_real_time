//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIGROUP_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIGROUP_H

#include "interface/ImGuiCaptionedElement.h"
#include "interface/ImGuiContainer.h"

namespace pf::ui {
class ImGuiGroup : public ImGuiContainer, public ImGuiCaptionedElement {
 public:
  ImGuiGroup(const std::string &elementName, const std::string &caption);

 protected:
  void renderImpl() override;
};
}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIGROUP_H
