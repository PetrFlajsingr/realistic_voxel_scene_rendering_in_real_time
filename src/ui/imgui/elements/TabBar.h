//
// Created by petr on 11/2/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_TABBAR_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_TABBAR_H

#include "interface/Container.h"
#include "interface/Element.h"
#include "interface/LabeledElement.h"

namespace pf::ui::ig {

class ImGuiTab : public LabeledElement, public Container {
 public:
  ImGuiTab(const std::string &elementName, const std::string &caption);

 protected:
  void renderImpl() override;
};

class TabBar : public Element {
 public:
  explicit TabBar(const std::string &elementName);

  std::shared_ptr<ImGuiTab> addTab(const std::string &name, const std::string &caption);
  void removeTab(const std::string &name);
 protected:
  void renderImpl() override;

 private:
  std::vector<std::shared_ptr<ImGuiTab>> tabs;
};

}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_TABBAR_H
