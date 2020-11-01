//
// Created by petr on 11/1/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIMENUBARS_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIMENUBARS_H

#include "interface/ImGuiCaptionedElement.h"
#include "interface/ImGuiElement.h"
#include <variant>
#include <vector>
#include <functional>

namespace pf::ui {

class ImGuiSubMenu;
class ImGuiMenuItem;

class ImGuiMenuContainer{
 public:
  ImGuiSubMenu &addSubmenu(const std::string &name, const std::string &caption);
  ImGuiMenuItem &addItem(const std::string &name, const std::string &caption);

  void removeItem(const std::string &name);
 protected:
  void renderItems();

 private:
  std::vector<std::variant<ImGuiMenuItem, ImGuiSubMenu>> items;
};


class ImGuiMenuItem : public ImGuiCaptionedElement {
 public:
  ImGuiMenuItem(const std::string &elementName, const std::string &caption);

  void setOnClick(std::invocable auto fnc) {
    onClick = fnc;
  }

 protected:
  void renderImpl() override;

 private:
  std::function<void()> onClick = []{};
};

class ImGuiSubMenu : public ImGuiCaptionedElement, public ImGuiMenuContainer {
 public:
  ImGuiSubMenu(const std::string &elementName, const std::string &caption);
 protected:
  void renderImpl() override;

};

class ImGuiWindowMenuBar : public ImGuiElement, public ImGuiMenuContainer {
 public:
  explicit ImGuiWindowMenuBar(const std::string &elementName);

 protected:
  void renderImpl() override;

};

class ImGuiAppMenuBar : public ImGuiElement, public ImGuiMenuContainer {
 public:
  explicit ImGuiAppMenuBar(const std::string &elementName);

 protected:
  void renderImpl() override;

};


}

#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIMENUBARS_H
