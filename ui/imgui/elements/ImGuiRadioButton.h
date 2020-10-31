//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIRADIOBUTTON_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIRADIOBUTTON_H

#include "interface/ImGuiCaptionedElement.h"
#include "interface/ImGuiResizableElement.h"
#include "interface/ImGuiValueObservableElement.h"
namespace pf::ui {

class ImGuiRadioButton : public ImGuiCaptionedElement, public ImGuiValueObservableElement<bool> {
 public:
  friend class ImGuiRadioGroup;
  ImGuiRadioButton(const std::string &elementName, const std::string &caption, bool value = false);

  [[nodiscard]] bool isSelected() const;

 protected:
  void renderImpl() override;

 private:
};

}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIRADIOBUTTON_H
