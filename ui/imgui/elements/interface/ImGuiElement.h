//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_IMGUIELEMENT_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_IMGUIELEMENT_H


#include <string>

namespace pf::ui {

class ImGuiElement {
 public:
  explicit ImGuiElement(std::string elementName);
  virtual void render() = 0;
  virtual ~ImGuiElement() = default;
  [[nodiscard]] const std::string &getName() const;

 private:
  const std::string name;
};

}
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_IMGUIELEMENT_H
