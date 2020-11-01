//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUITREE_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUITREE_H

#include "interface/ImGuiCaptionedElement.h"
#include "interface/ImGuiContainer.h"
#include <vector>

namespace pf::ui {

class ImGuiTree : public ImGuiCaptionedElement, public ImGuiContainer {
 public:
  ImGuiTree(const std::string &elementName, const std::string &caption);

  std::shared_ptr<ImGuiTree> addNode(const std::string &elementName, const std::string &caption);

 protected:
  void renderImpl() override;
};

}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUITREE_H
