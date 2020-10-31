//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUITEXT_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUITEXT_H

#include "interface/ImGuiElement.h"

namespace pf::ui {
class ImGuiText : public virtual ImGuiElement {
 public:
  ImGuiText(const std::string &elementName, std::string text);
  void render() override;
  [[nodiscard]] const std::string &getText() const;
  void setText(const std::string &text);

 private:
  std::string text;
};

}
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUITEXT_H
