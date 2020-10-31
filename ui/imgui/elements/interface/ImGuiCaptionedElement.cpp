//
// Created by petr on 10/31/20.
//

#include "ImGuiCaptionedElement.h"

#include <utility>

namespace pf::ui {
ImGuiCaptionedElement::ImGuiCaptionedElement(std::string elementName,
                                             std::string caption)
    : ImGuiElement(std::move(elementName)), caption(std::move(caption)) {}

const std::string &ImGuiCaptionedElement::getCaption() const { return caption; }

void ImGuiCaptionedElement::setCaption(const std::string &cap) {
  caption = cap;
}
}