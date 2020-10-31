//
// Created by petr on 10/31/20.
//

#include "ImGuiText.h"
#include <imgui.h>

#include <cassert>
#include <utility>

namespace pf::ui {

ImGuiText::ImGuiText(const std::string &elementName, std::string text)
    : ImGuiElement(elementName), text(std::move(text)) {}

const std::string &ImGuiText::getText() const { return text; }
void ImGuiText::setText(const std::string &txt) { text = txt; }

void ImGuiText::render() { ImGui::Text("%s", text.c_str()); }
}// namespace pf::ui