//
// Created by petr on 11/1/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUILISTBOX_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUILISTBOX_H

#include "interface/ImGuiCaptionedElement.h"
#include "interface/ImGuiValueObservableElement.h"
#include <vector>

namespace pf::ui {
class ImGuiListBox : public ImGuiCaptionedElement,
                     public ImGuiValueObservableElement<std::string_view> {
 public:
  ImGuiListBox(const std::string &elementName, const std::string &caption,
               std::vector<std::string> items_ = {}, int selectedIdx = 0, int heightInItems = -1);

  void addItem(std::string item);

  [[nodiscard]] std::string_view getSelectedItem() const;

 protected:
  void renderImpl() override;

 private:
  std::vector<std::string> items;
  int currentItemIdx = 0;
  int height = -1;
};
}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUILISTBOX_H
