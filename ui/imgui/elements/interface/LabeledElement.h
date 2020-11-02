//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICAPTIONEDELEMENT_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICAPTIONEDELEMENT_H

#include "Element.h"

namespace pf::ui::ig {
class LabeledElement : public virtual Element {
 public:
  LabeledElement(std::string elementName, std::string caption);

  [[nodiscard]] const std::string &getLabel() const;
  void setLabel(const std::string &cap);

 private:
  std::string label;
};

}
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICAPTIONEDELEMENT_H
