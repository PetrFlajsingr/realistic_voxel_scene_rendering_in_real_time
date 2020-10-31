//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIINPUT_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIINPUT_H

#include "../concepts/OneOf.h"
#include "interface/ImGuiCaptionedElement.h"
#include "interface/ImGuiValueObservableElement.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <utility>

namespace pf::ui {
namespace details {
#define IMGUI_INPUT_STEP_TYPE_LIST float, double, int
#define IMGUI_INPUT_FLOAT_TYPE_LIST float, glm::vec2, glm::vec3, glm::vec4
#define IMGUI_INPUT_DOUBLE_TYPE_LIST double
#define IMGUI_INPUT_INT_TYPE_LIST int, glm::ivec2, glm::ivec3, glm::ivec4
#define IMGUI_INPUT_TYPE_LIST                                                                      \
  IMGUI_INPUT_FLOAT_TYPE_LIST, IMGUI_INPUT_INT_TYPE_LIST, IMGUI_INPUT_DOUBLE_TYPE_LIST

template<OneOf<IMGUI_INPUT_TYPE_LIST> T>
using InputUnderlyingType =
    std::conditional_t<OneOf<T, IMGUI_INPUT_FLOAT_TYPE_LIST>, float,
                       std::conditional_t<OneOf<T, IMGUI_INPUT_INT_TYPE_LIST>, int, double>>;

template<typename T>
struct InputData {};

template<>
struct InputData<int> {
  int step;
  int fastStep;
};
template<>
struct InputData<float> {
  float step;
  float fastStep;
  static constexpr const char *defaultFormat() { return "%.3f"; }
};
template<>
struct InputData<double> {
  double step;
  double fastStep;
  static constexpr const char *defaultFormat() { return "%.6f"; }
};

template<typename T>
concept UnformattedWithStep =
    OneOf<
        T,
        IMGUI_INPUT_STEP_TYPE_LIST> && !OneOf<T, IMGUI_INPUT_FLOAT_TYPE_LIST, IMGUI_INPUT_DOUBLE_TYPE_LIST>;

template<typename T>
concept UnformattedWithoutStep =
    !OneOf<
        T,
        IMGUI_INPUT_STEP_TYPE_LIST> && !OneOf<T, IMGUI_INPUT_FLOAT_TYPE_LIST, IMGUI_INPUT_DOUBLE_TYPE_LIST>;

template<typename T>
concept FormattedWithStep = OneOf<T, IMGUI_INPUT_STEP_TYPE_LIST>
    &&OneOf<T, IMGUI_INPUT_FLOAT_TYPE_LIST, IMGUI_INPUT_DOUBLE_TYPE_LIST>;
template<typename T>
concept FormattedWithoutStep =
    !OneOf<
        T,
        IMGUI_INPUT_STEP_TYPE_LIST> && OneOf<T, IMGUI_INPUT_FLOAT_TYPE_LIST, IMGUI_INPUT_DOUBLE_TYPE_LIST>;
}// namespace details

template<OneOf<IMGUI_INPUT_TYPE_LIST> T>
class ImGuiInput : public ImGuiCaptionedElement, public ImGuiValueObservableElement<T> {
  details::InputData<details::InputUnderlyingType<T>> data;

 public:
  ImGuiInput(const std::string &elementName, const std::string &caption, T st = 0, T fStep = 0,
             T value = T{}) requires details::UnformattedWithStep<T>
      : ImGuiElement(elementName),
        ImGuiCaptionedElement(elementName, caption),
        ImGuiValueObservableElement<T>(elementName, value),
        data(st, fStep) {}

  ImGuiInput(const std::string &elementName, const std::string &caption, T st = 0, T fStep = 0,
             std::string format = decltype(data)::defaultFormat(),
             T value = T{}) requires details::FormattedWithStep<T>
      : ImGuiElement(elementName),
        ImGuiCaptionedElement(elementName, caption),
        ImGuiValueObservableElement<T>(elementName, value),
        format(std::move(format)),
        data(st, fStep) {}

  ImGuiInput(const std::string &elementName, const std::string &caption,
             T value = T{}) requires details::UnformattedWithoutStep<T>
      : ImGuiElement(elementName),
        ImGuiCaptionedElement(elementName, caption),
        ImGuiValueObservableElement<T>(elementName, value) {}

  ImGuiInput(const std::string &elementName, const std::string &caption,
             std::string format = decltype(data)::defaultFormat(),
             T value = T{}) requires details::FormattedWithoutStep<T>
      : ImGuiElement(elementName),
        ImGuiCaptionedElement(elementName, caption),
        ImGuiValueObservableElement<T>(elementName, value),
        format(std::move(format)) {}

  void render() override {
    const auto oldValue = ImGuiValueObservableElement<T>::getValue();
    if constexpr (std::same_as<T, float>) {
      ImGui::InputFloat(getCaption().c_str(), ImGuiValueObservableElement<T>::getValueAddress(),
                        data.step, data.fastStep, format.c_str());
    }
    if constexpr (std::same_as<T, glm::vec2>) {
      ImGui::InputFloat2(getCaption().c_str(),
                         glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()));
    }
    if constexpr (std::same_as<T, glm::vec3>) {
      ImGui::InputFloat3(getCaption().c_str(),
                         glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()));
    }
    if constexpr (std::same_as<T, glm::vec4>) {
      ImGui::InputFloat4(getCaption().c_str(),
                         glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()));
    }
    if constexpr (std::same_as<T, int>) {
      ImGui::InputInt(getCaption().c_str(), ImGuiValueObservableElement<T>::getValueAddress(),
                      data.step, data.fastStep);
    }
    if constexpr (std::same_as<T, glm::ivec2>) {
      ImGui::InputInt2(getCaption().c_str(),
                       glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()));
    }
    if constexpr (std::same_as<T, glm::ivec3>) {
      ImGui::InputInt3(getCaption().c_str(),
                       glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()));
    }
    if constexpr (std::same_as<T, glm::ivec4>) {
      ImGui::InputInt4(getCaption().c_str(),
                       glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()));
    }
    if constexpr (std::same_as<T, double>) {
      ImGui::InputDouble(getCaption().c_str(),
                         glm::value_ptr(*ImGuiValueObservableElement<T>::getValueAddress()),
                         data.step, data.fastStep, format.c_str());
    }
    if (oldValue != ImGuiValueObservableElement<T>::getValue()) {
      ImGuiValueObservableElement<T>::notifyValueChanged();
    }
  }

 private:
  std::string format;
};
}// namespace pf::ui

#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIINPUT_H
