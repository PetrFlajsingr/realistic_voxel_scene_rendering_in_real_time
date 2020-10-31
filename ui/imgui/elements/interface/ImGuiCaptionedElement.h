//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICAPTIONEDELEMENT_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICAPTIONEDELEMENT_H

#include "ImGuiElement.h"

namespace pf::ui {
class ImGuiCaptionedElement : public virtual ImGuiElement {
 public:
  ImGuiCaptionedElement(std::string elementName, std::string caption);

  [[nodiscard]] const std::string &getCaption() const;
  void setCaption(const std::string &cap);

 private:
  std::string caption;
};

}
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICAPTIONEDELEMENT_H
