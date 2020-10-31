//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIPROGRESSBAR_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIPROGRESSBAR_H

#include "interface/ImGuiResizableElement.h"
#include "interface/ImGuiValueObservableElement.h"
#include <imgui.h>

namespace pf::ui {

template<typename T>
concept ProgressBarCompatible = requires(T t, float f) {
  {t + t};
  {t *= f};
  { std::clamp(t, t, t) }
  ->std::convertible_to<T>;
};

template<ProgressBarCompatible T>
class ImGuiProgressBar : public ImGuiValueObservableElement<T>, public ImGuiResizableElement {
 public:
  ImGuiProgressBar(const std::string &elementName, T stepValue, T min, T max,
                   std::optional<T> value = std::nullopt, const ImVec2 &size = {-1, 0})
      : ImGuiElement(elementName), ImGuiValueObservableElement<T>(elementName,
                                                                  value.has_value() ? *value : min),
        ImGuiResizableElement(elementName, size), stepValue(stepValue), min(min), max(max) {}

  T setPercentage(float percentage) {
    percentage = std::clamp(percentage, 0.f, 1.f);
    const auto oldValue = ImGuiValueObservableElement<T>::getValue();
    const auto newValue = min + (max - min) * percentage;
    ImGuiValueObservableElement<T>::setValue(newValue);
    if (ImGuiValueObservableElement<T>::getValue() != oldValue) {
      ImGuiValueObservableElement<T>::notifyValueChanged();
    }
    return newValue;
  }

  T step() {
    const auto oldValue = ImGuiValueObservableElement<T>::getValue();
    const auto newValue = std::clamp(oldValue + stepValue, min, max);
    ImGuiValueObservableElement<T>::setValue(newValue);
    if (ImGuiValueObservableElement<T>::getValue() != oldValue) {
      ImGuiValueObservableElement<T>::notifyValueChanged();
    }
    return newValue;
  }

  float getCurrentPercentage() {
    const auto size = max - min;
    return (ImGuiValueObservableElement<T>::getValue() - min) / static_cast<float>(size);
  }

 protected:
  void renderImpl() override { ImGui::ProgressBar(getCurrentPercentage(), getSize()); }

 private:
  T stepValue;
  T min;
  T max;
};
}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIPROGRESSBAR_H
