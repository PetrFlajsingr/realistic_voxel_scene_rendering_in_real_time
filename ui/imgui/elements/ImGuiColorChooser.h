//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICOLORCHOOSER_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICOLORCHOOSER_H

#include "../concepts/OneOf.h"
#include "interface/ImGuiCaptionedElement.h"
#include "interface/ImGuiValueObservableElement.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace pf::ui {

enum class ColorChooserType { Edit, Picker };

template<ColorChooserType Type, OneOf<glm::vec3, glm::vec4> T>
class ImGuiColorChooser : public ImGuiCaptionedElement, public ImGuiValueObservableElement<T> {
 public:
  ImGuiColorChooser(const std::string &elementName, const std::string &caption, T value = T{})
      : ImGuiElement(elementName),
        ImGuiCaptionedElement(elementName, caption), ImGuiValueObservableElement<T>(elementName,
                                                                                    value) {}

  void render() override {
    const auto oldValue = ImGuiValueObservableElement<T>::getValue();
    if constexpr (Type == ColorChooserType::Edit) {
      if constexpr (std::same_as<glm::vec3, T>) {
        ImGui::ColorEdit3(getCaption().c_str(),
                          glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()));
      } else {
        ImGui::ColorEdit4(getCaption().c_str(),
                          glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()));
      }
    } else {
      if constexpr (std::same_as<glm::vec3, T>) {
        ImGui::ColorPicker3(getCaption().c_str(),
                            glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()));
      } else {
        ImGui::ColorPicker4(getCaption().c_str(),
                            glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()));
      }
    }
    if (oldValue != ImGuiValueObservableElement<T>::getValue()) {
      ImGuiValueObservableElement<T>::notifyValueChanged();
    }
  }

 private:
};

}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICOLORCHOOSER_H
