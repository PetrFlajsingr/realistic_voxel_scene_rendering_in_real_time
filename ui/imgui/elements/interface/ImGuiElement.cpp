//
// Created by petr on 10/31/20.
//

#include "ImGuiElement.h"

namespace pf::ui {

ImGuiElement::ImGuiElement(std::string elementName) : name(std::move(elementName)) {}
const std::string &ImGuiElement::getName() const { return name; }

}