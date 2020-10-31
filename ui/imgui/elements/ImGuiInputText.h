//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIINPUTTEXT_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIINPUTTEXT_H

#include "ImGuiText.h"
#include "interface/ImGuiCaptionedElement.h"
#include "interface/ImGuiValueObservableElement.h"
#include <functional>
namespace pf::ui {
class ImGuiInputText : public ImGuiText,
                       public ImGuiCaptionedElement,
                       public ImGuiValueObservableElement<std::string_view> {
 public:
  ImGuiInputText(const std::string &elementName, std::string caption, const std::string &text = "");
  void render() override;

 private:
  char buffer[256]{};
};

}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIINPUTTEXT_H
