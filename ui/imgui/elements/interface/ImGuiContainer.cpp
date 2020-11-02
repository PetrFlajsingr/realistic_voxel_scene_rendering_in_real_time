//
// Created by petr on 10/31/20.
//

#include "ImGuiContainer.h"
#include "../../../../exceptions/StackTraceException.h"

namespace pf::ui {

ImGuiContainer::ImGuiContainer(const std::string &elementName) : ImGuiElement(elementName) {}

void ImGuiContainer::addChild(std::shared_ptr<ImGuiElement> child) {
  if (const auto iter = children.find(child->getName()); iter != children.end()) {
    throw StackTraceException::fmt("{} already present in ui", child->getName());
  }
  childrenInOrder.emplace_back(*child);
  children[child->getName()] = std::move(child);
}

void ImGuiContainer::removeChild(const std::string &name) {
  if (const auto iter = children.find(name); iter != children.end()) {
    auto ptr = iter->second.get();
    auto ptrIter =
        std::ranges::find_if(childrenInOrder, [ptr](auto &child) { return &child.get() == ptr; });
    childrenInOrder.erase(ptrIter);
    children.erase(iter);
  }
}
const std::vector<std::reference_wrapper<ImGuiElement>> &ImGuiContainer::getChildren() {
  std::ranges::for_each(childrenToRemove, [this](const auto &name) { removeChild(name); });
  childrenToRemove.clear();
  return childrenInOrder;
}
void ImGuiContainer::enqueueChildRemoval(const std::string &name) {
  childrenToRemove.emplace_back(name);
}
void ImGuiContainer::clear() {
  childrenInOrder.clear();
  children.clear();
  childrenToRemove.clear();
}
}// namespace pf::ui