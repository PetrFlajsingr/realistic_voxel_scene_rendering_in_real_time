//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIRADIOGROUP_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIRADIOGROUP_H
#include "ImGuiRadioButton.h"
#include "interface/ImGuiCaptionedElement.h"
namespace pf::ui {

class ImGuiRadioGroup : public ImGuiCaptionedElement, public ImGuiValueObservableElement<std::string_view> {
 public:
  ImGuiRadioGroup(const std::string &elementName, const std::string &caption,
                  std::vector<ImGuiRadioButton> buttons = {},
                  const std::optional<std::size_t> &selectedButtonIndex = std::nullopt);

  void render() override;
  void addButton(const std::string &elementName, const std::string &caption, bool value = false);

 private:
  std::vector<ImGuiRadioButton> buttons;
  std::optional<std::size_t> selectedButtonIndex = std::nullopt;
};
}

#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIRADIOGROUP_H
