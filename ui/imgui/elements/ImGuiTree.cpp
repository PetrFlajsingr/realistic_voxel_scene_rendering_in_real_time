//
// Created by petr on 10/31/20.
//

#include "ImGuiTree.h"
#include <imgui.h>

namespace pf::ui {

ImGuiTree::ImGuiTree(const std::string &elementName, const std::string &caption)
    : ImGuiElement(elementName), ImGuiCaptionedElement(elementName, caption),
      ImGuiContainer(elementName) {}

std::shared_ptr<ImGuiTree> ImGuiTree::addNode(const std::string &elementName,
                                              const std::string &caption) {
  return createChild<ImGuiTree>(elementName, caption);
}

void ImGuiTree::renderImpl() {
  if (ImGui::TreeNode(getCaption().c_str())) {
    std::ranges::for_each(getChildren(), [](auto &child) { child.get().render(); });
    ImGui::TreePop();
  }
}
}// namespace pf::ui