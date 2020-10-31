//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIRESIZABLEELEMENT_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIRESIZABLEELEMENT_H

#include "ImGuiElement.h"
#include <imgui.h>
namespace pf::ui {
class ImGuiResizableElement : public virtual ImGuiElement {
 public:
  ImGuiResizableElement(std::string elementName, const ImVec2 &size);

  [[nodiscard]] const ImVec2 &getSize() const;
  void setSize(const ImVec2 &s);

 private:
  ImVec2 size;
};

}
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIRESIZABLEELEMENT_H
