//
// Created by petr on 11/2/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_SERIALIZATION_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_SERIALIZATION_H

#include "elements.h"
#include "ui_tree_traversal.h"
#include <toml++/toml.h>

namespace pf::ui::ig {

inline toml::table serializeImGuiTree(Element &root) {
  auto result = toml::table();
  traverseImGuiTree(root, [&result](Element &element) {
    if (auto ptrSavable = dynamic_cast<SavableElement *>(&element); ptrSavable != nullptr) {
      const auto optSerialised = ptrSavable->serialize();
      if (optSerialised.has_value()) {
        result.insert_or_assign(ptrSavable->getName(), *optSerialised);
      }
    }
  });
  return result;
}

}// namespace pf::ui

#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_SERIALIZATION_H
