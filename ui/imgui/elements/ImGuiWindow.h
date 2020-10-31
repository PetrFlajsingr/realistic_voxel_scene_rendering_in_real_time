//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIWINDOW_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIWINDOW_H

#include "interface/ImGuiContainer.h"
#include "interface/ImGuiResizableElement.h"

namespace pf::ui {
// TODO: resize, focus, collapse, position
class ImGuiWindow : public ImGuiContainer {
 public:
  ImGuiWindow(const std::string &elementName, std::string title);

  [[nodiscard]] const std::string &getTitle() const;
  void setTitle(const std::string &title);

 protected:
  void renderImpl() override;

 private:
  std::string title;
};

}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIWINDOW_H
