//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_INTERFACE_IMGUIVALUEOBSERVABLEELEMENT_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_INTERFACE_IMGUIVALUEOBSERVABLEELEMENT_H

#include "../../../../coroutines/Sequence.h"
#include "../../../events/Subscription.h"
#include "ImGuiElement.h"
#include <algorithm>
#include <functional>
#include <ranges>

namespace pf::ui {
template<typename T>
class ImGuiValueObservableElement : public virtual ImGuiElement {
 public:
  using Callback = std::function<void(const T &)>;
  using Id = uint32_t;
  explicit ImGuiValueObservableElement(const std::string &elementName, T value = T{})
      : ImGuiElement(elementName), value(value) {}

  events::Subscription addValueListener(std::invocable<const T &> auto fnc) {
    const auto id = generateListenerId();
    listeners[id] = fnc;
    return events::Subscription([id, this] { listeners.erase(id); });
  }

  [[nodiscard]] const T &getValue() const { return value; }

 protected:
  void notifyValueChanged() {
    auto callables = listeners | std::views::values;
    std::ranges::for_each(callables, [&](auto &callable) { callable(value); });
  }
  void setValue(T val) { value = val; }

  T *getValueAddress() { return &value; }

 private:
  Id generateListenerId() { return getNext(idGenerator); }

  T value;
  std::unordered_map<Id, Callback> listeners;

  cppcoro::generator<Id> idGenerator = iota<Id>();
};

}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_INTERFACE_IMGUIVALUEOBSERVABLEELEMENT_H
