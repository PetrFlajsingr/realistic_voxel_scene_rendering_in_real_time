//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIDRAGINPUT_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIDRAGINPUT_H

#include "../concepts/OneOf.h"
#include "../utils/math.h"
#include "interface/ImGuiCaptionedElement.h"
#include "interface/ImGuiValueObservableElement.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <utility>

namespace pf::ui {
namespace details {
#define IMGUI_DRAG_FLOAT_TYPE_LIST float, glm::vec2, glm::vec3, glm::vec4, math::Range<float>
#define IMGUI_DRAG_INT_TYPE_LIST int, glm::ivec2, glm::ivec3, glm::ivec4, math::Range<int>
#define IMGUI_DRAG_RANGE_TYPE_LIST math::Range<int>, math::Range<float>
#define IMGUI_DRAG_TYPE_LIST IMGUI_DRAG_FLOAT_TYPE_LIST, IMGUI_DRAG_INT_TYPE_LIST

template<typename T>
using UnderlyingType = std::conditional_t<OneOf<T, IMGUI_DRAG_FLOAT_TYPE_LIST>, float, int>;

template<typename T>
constexpr const char *defaultDragFormat() {
  if constexpr (OneOf<T, IMGUI_DRAG_FLOAT_TYPE_LIST> || std::same_as<T, math::Range<float>>) {
    return "%.3f";
  } else {
    return "%d";
  }
}
}// namespace details
template<OneOf<IMGUI_DRAG_TYPE_LIST> T>
class ImGuiDragInput : public ImGuiValueObservableElement<T>, public ImGuiCaptionedElement {
 public:
  using ParamType = details::UnderlyingType<T>;

  ImGuiDragInput(const std::string &elementName, const std::string &caption, ParamType speed,
                 ParamType min, ParamType max, T value = T{},
                 std::string format = details::defaultDragFormat<T>())
      : ImGuiElement(elementName), ImGuiValueObservableElement<T>(elementName, value),
        ImGuiCaptionedElement(elementName, caption), speed(speed), min(min), max(max),
        format(std::move(format)) {}

 protected:
  void renderImpl() override {
    const auto oldValue = ImGuiValueObservableElement<T>::getValue();
    if constexpr (std::same_as<T, float>) {
      ImGui::DragFloat(getCaption().c_str(), ImGuiValueObservableElement<T>::getValueAddress(),
                       speed, min, max, format.c_str());
    }
    if constexpr (std::same_as<T, glm::vec2>) {
      ImGui::DragFloat2(getCaption().c_str(),
                        glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()), speed,
                        min, max, format.c_str());
    }
    if constexpr (std::same_as<T, glm::vec3>) {
      ImGui::DragFloat3(getCaption().c_str(),
                        glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()), speed,
                        min, max, format.c_str());
    }
    if constexpr (std::same_as<T, glm::vec4>) {
      ImGui::DragFloat4(getCaption().c_str(),
                        glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()), speed,
                        min, max, format.c_str());
    }
    if constexpr (std::same_as<T, int>) {
      ImGui::DragInt(getCaption().c_str(), ImGuiValueObservableElement<T>::getValueAddress(), speed,
                     min, max, format.c_str());
    }
    if constexpr (std::same_as<T, glm::ivec2>) {
      ImGui::DragInt2(getCaption().c_str(),
                      glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()), speed,
                      min, max, format.c_str());
    }
    if constexpr (std::same_as<T, glm::ivec3>) {
      ImGui::DragInt3(getCaption().c_str(),
                      glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()), speed,
                      min, max, format.c_str());
    }
    if constexpr (std::same_as<T, glm::ivec4>) {
      ImGui::DragInt4(getCaption().c_str(),
                      glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()), speed,
                      min, max, format.c_str());
    }
    if constexpr (std::same_as<T, math::Range<int>>) {
      ImGui::DragIntRange2(
          getCaption().c_str(),
          reinterpret_cast<int *>(ImGuiValueObservableElement<T>::getValueAddress()), speed, min,
          max, format.c_str());
    }
    if constexpr (std::same_as<T, math::Range<float>>) {
      ImGui::DragFloatRange2(
          getCaption().c_str(), &ImGuiValueObservableElement<T>::getValueAddress()->start,
          &ImGuiValueObservableElement<T>::getValueAddress()->end, speed, min, max, format.c_str());
    }
    if (oldValue != ImGuiValueObservableElement<T>::getValue()) {
      ImGuiValueObservableElement<T>::notifyValueChanged();
    }
  }

 private:
  ParamType speed;
  ParamType min;
  ParamType max;
  std::string format;
};

}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIDRAGINPUT_H
