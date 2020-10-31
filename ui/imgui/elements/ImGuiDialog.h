//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIDIALOG_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIDIALOG_H

#include "interface/ImGuiCaptionedElement.h"
#include "interface/ImGuiContainer.h"

namespace pf::ui {

enum class Modal {
  Yes, No
};

class ImGuiDialog : public ImGuiContainer, public ImGuiCaptionedElement {
 public:
  ImGuiDialog(ImGuiContainer &parent, const std::string &elementName,
              const std::string &caption, Modal modal = Modal::Yes);
  void render() override;

  void close();
 private:
  Modal modal;
  bool closed = false;
  ImGuiContainer &owner;
};
}
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIDIALOG_H
