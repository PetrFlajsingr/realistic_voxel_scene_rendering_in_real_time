//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_IMGUIELEMENT_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_IMGUIELEMENT_H


#include <string>

namespace pf::ui {

enum class Visibility {
  Visible, Invisible
};

class ImGuiElement {
 public:
  explicit ImGuiElement(std::string elementName);
  virtual ~ImGuiElement() = default;
  [[nodiscard]] const std::string &getName() const;

  [[nodiscard]] Visibility getVisibility() const;
  void setVisibility(Visibility visibility);

  void render();

 protected:
  virtual void renderImpl() = 0;

 private:
  const std::string name;
  Visibility visibility = Visibility::Visible;
};

}
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_IMGUIELEMENT_H
