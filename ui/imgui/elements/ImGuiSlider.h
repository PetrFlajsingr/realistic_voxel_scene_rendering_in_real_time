//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUISLIDER_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUISLIDER_H

#include "../concepts/OneOf.h"
#include "interface/ImGuiCaptionedElement.h"
#include "interface/ImGuiValueObservableElement.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <utility>

namespace pf::ui {
//TODO: angle

namespace details {
#define IMGUI_SLIDER_FLOAT_TYPE_LIST float, glm::vec2, glm::vec3, glm::vec4
#define IMGUI_SLIDER_INT_TYPE_LIST int, glm::uvec2, glm::uvec3, glm::uvec4
#define IMGUI_SLIDER_TYPE_LIST IMGUI_SLIDER_FLOAT_TYPE_LIST, IMGUI_SLIDER_INT_TYPE_LIST
template<OneOf<IMGUI_SLIDER_TYPE_LIST> T>
using SliderMinMaxType = std::conditional_t<OneOf<T, IMGUI_SLIDER_FLOAT_TYPE_LIST>, float, int>;

template<typename T>
constexpr const char *defaultSliderFormat() {
  if constexpr (OneOf<T, IMGUI_SLIDER_FLOAT_TYPE_LIST>) {
    return "%.3f";
  } else {
    return "%d";
  }
}
}// namespace details
template<OneOf<IMGUI_SLIDER_TYPE_LIST> T>
class ImGuiSlider : public ImGuiCaptionedElement, public ImGuiValueObservableElement<T> {
 public:
  using MinMaxType = details::SliderMinMaxType<T>;
  ImGuiSlider(const std::string &elementName, const std::string &caption, MinMaxType min,
              MinMaxType max, T value = T{},
              std::string format = details::defaultSliderFormat<MinMaxType>())
      : ImGuiElement(elementName),
        ImGuiCaptionedElement(elementName, caption), ImGuiValueObservableElement<T>(elementName,
                                                                                    value),
        min(min), max(max), format(std::move(format)) {}

  void render() override {
    const auto oldValue = ImGuiValueObservableElement<T>::getValue();
    if constexpr (std::same_as<T, float>) {
      ImGui::SliderFloat(getCaption().c_str(), ImGuiValueObservableElement<T>::getValueAddress(),
                         min, max, format.c_str());
    }
    if constexpr (std::same_as<T, glm::vec2>) {
      ImGui::SliderFloat2(getCaption().c_str(),
                          glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()), min,
                          max, format.c_str());
    }
    if constexpr (std::same_as<T, glm::vec3>) {
      ImGui::SliderFloat3(getCaption().c_str(),
                          glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()), min,
                          max, format.c_str());
    }
    if constexpr (std::same_as<T, glm::vec4>) {
      ImGui::SliderFloat4(getCaption().c_str(),
                          glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()), min,
                          max, format.c_str());
    }
    if constexpr (std::same_as<T, int>) {
      ImGui::SliderInt(getCaption().c_str(), ImGuiValueObservableElement<T>::getValueAddress(), min,
                       max, format.c_str());
    }
    if constexpr (std::same_as<T, glm::uvec2>) {
      ImGui::SliderInt2(getCaption().c_str(),
                        glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()), min,
                        max, format.c_str());
    }
    if constexpr (std::same_as<T, glm::uvec3>) {
      ImGui::SliderInt3(getCaption().c_str(),
                        glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()), min,
                        max, format.c_str());
    }
    if constexpr (std::same_as<T, glm::uvec4>) {
      ImGui::SliderInt4(getCaption().c_str(),
                        glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()), min,
                        max, format.c_str());
    }
    if (oldValue != ImGuiValueObservableElement<T>::getValue()) {
      ImGuiValueObservableElement<T>::notifyValueChanged();
    }
  }

 private:
  MinMaxType min;
  MinMaxType max;
  std::string format;
};

}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUISLIDER_H
