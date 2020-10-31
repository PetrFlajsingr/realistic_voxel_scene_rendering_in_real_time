//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICHECKBOX_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICHECKBOX_H

#include "interface/ImGuiCaptionedElement.h"
#include "interface/ImGuiValueObservableElement.h"

namespace pf::ui {
class ImGuiCheckbox : public ImGuiValueObservableElement<bool>, public ImGuiCaptionedElement {
 public:
  ImGuiCheckbox(const std::string &elementName, const std::string &caption, bool value = false);
 protected:
  void renderImpl() override;
};
}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICHECKBOX_H
