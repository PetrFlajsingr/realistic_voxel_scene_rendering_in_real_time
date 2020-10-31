//
// Created by petr on 10/31/20.
//

#include "ImGuiDialog.h"
#include <imgui.h>
namespace pf::ui {
ImGuiDialog::ImGuiDialog(ImGuiContainer &parent, const std::string &elementName, const std::string &caption, Modal modal)
    : ImGuiElement(elementName), ImGuiContainer(elementName),
      ImGuiCaptionedElement(elementName, caption), modal(modal), owner(parent) {}

void ImGuiDialog::render() {
  if (closed) {
    owner.enqueueChildRemoval(getName());
    return;
  }
  ImGui::OpenPopup(getCaption().c_str());
  auto open = bool{};
  if (modal == Modal::Yes) {
    open = ImGui::BeginPopupModal(getCaption().c_str());
  } else {
    open = ImGui::BeginPopup(getCaption().c_str());
  }
  if (open) {
    std::ranges::for_each(getChildren(), [](auto &child) { child.second->render(); });
    ImGui::EndPopup();
  }
  if (closed) {
    owner.enqueueChildRemoval(getName());
  }
}
void ImGuiDialog::close() {
  closed = true;
}

}// namespace pf::ui