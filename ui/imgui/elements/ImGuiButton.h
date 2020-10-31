//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIBUTTON_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIBUTTON_H

#include "interface/ImGuiCaptionedElement.h"
#include "interface/ImGuiResizableElement.h"
#include <functional>
#include <imgui.h>
namespace pf::ui {

class ImGuiButton : public ImGuiCaptionedElement, public ImGuiResizableElement {
 public:
  ImGuiButton(const std::string &name, std::string caption, const ImVec2 &size = {0, 0});
  void render() override;

  void setOnClick(std::invocable auto fnc) { onClick = fnc; }

 private:
  std::function<void()> onClick = [] {};
};

}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIBUTTON_H
