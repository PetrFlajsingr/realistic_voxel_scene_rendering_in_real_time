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
  children[child->getName()] = std::move(child);
}

void ImGuiContainer::removeChild(const std::string &name) { children.erase(name); }
const std::map<std::string, std::shared_ptr<ImGuiElement>> &ImGuiContainer::getChildren()  {
  std::ranges::for_each(childrenToRemove, [this] (const auto &name) {
    removeChild(name);
  });
  childrenToRemove.clear();
  return children;
}
void ImGuiContainer::enqueueChildRemoval(const std::string &name) {
  childrenToRemove.emplace_back(name);
}
}// namespace pf::ui