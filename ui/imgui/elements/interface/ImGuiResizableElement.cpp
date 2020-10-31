//
// Created by petr on 10/31/20.
//

#include "ImGuiResizableElement.h"

#include <utility>

namespace pf::ui {

ImGuiResizableElement::ImGuiResizableElement(std::string elementName, const ImVec2 &size)
    : ImGuiElement(std::move(elementName)), size(size) {}

const ImVec2 &ImGuiResizableElement::getSize() const { return size; }

void ImGuiResizableElement::setSize(const ImVec2 &s) { size = s; }
}// namespace pf::ui