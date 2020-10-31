//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICOMBOBOX_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICOMBOBOX_H

#include "interface/ImGuiCaptionedElement.h"
#include "interface/ImGuiValueObservableElement.h"
#include <optional>
#include <vector>

namespace pf::ui {
class ImGuiComboBox : public ImGuiCaptionedElement,
                      public ImGuiValueObservableElement<std::string_view> {
 public:
  ImGuiComboBox(const std::string &elementName, const std::string &caption,
                std::string previewValue, std::vector<std::string> items);

  [[nodiscard]] std::optional<std::string_view> getSelectedItem();
  void removeItem(const std::string &item);
  void addItem(const std::string &item);

 protected:
  void renderImpl() override;

 private:
  std::vector<std::string> items;
  std::string previewValue;
  std::optional<uint> selectedItemIndex = std::nullopt;
};
}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICOMBOBOX_H
