//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUITEXT_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUITEXT_H

#include "interface/ImGuiElement.h"

namespace pf::ui {
// TODO: color
class ImGuiText : public virtual ImGuiElement {
 public:
  ImGuiText(const std::string &elementName, std::string text);

  [[nodiscard]] const std::string &getText() const;
  void setText(const std::string &text);

 protected:
  void renderImpl() override;

 private:
  std::string text;
};

}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUITEXT_H
