//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICONTAINER_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICONTAINER_H

#include "ImGuiElement.h"
#include <map>
#include <memory>
#include <string>

namespace pf::ui {
class ImGuiContainer : public ImGuiElement {
 public:
  explicit ImGuiContainer(const std::string &elementName);
  template<typename T, typename... Args>
  requires std::derived_from<T, ImGuiElement> &&std::constructible_from<T, std::string, Args...>
      std::shared_ptr<T> createChild(std::string name, Args &&... args) {
    auto child = std::make_shared<T>(name, std::forward<Args>(args)...);
    addChild(child);
    return child;
  }
  void addChild(std::shared_ptr<ImGuiElement> child);
  void removeChild(const std::string &name);

  [[nodiscard]] const std::map<std::string, std::shared_ptr<ImGuiElement>> &getChildren() const;

 private:
  std::map<std::string, std::shared_ptr<ImGuiElement>> children;
};

}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUICONTAINER_H
