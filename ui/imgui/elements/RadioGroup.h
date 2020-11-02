//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_RADIOGROUP_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_RADIOGROUP_H
#include "RadioButton.h"
#include "interface/LabeledElement.h"
namespace pf::ui::ig {

class RadioGroup : public LabeledElement,
                        public ValueObservableElement<std::string_view> {
 public:
  RadioGroup(const std::string &elementName, const std::string &caption,
                  std::vector<RadioButton> buttons = {},
                  const std::optional<std::size_t> &selectedButtonIndex = std::nullopt);

  void addButton(const std::string &elementName, const std::string &caption, bool value = false);

 protected:
  void renderImpl() override;

 private:
  std::vector<RadioButton> buttons;
  std::optional<std::size_t> selectedButtonIndex = std::nullopt;
};
}// namespace pf::ui

#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_RADIOGROUP_H
