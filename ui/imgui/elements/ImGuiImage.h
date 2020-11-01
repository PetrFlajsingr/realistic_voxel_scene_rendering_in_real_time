//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIIMAGE_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIIMAGE_H

#include "interface/ImGuiResizableElement.h"
namespace pf::ui {

class ImGuiImage : public ImGuiResizableElement {
 public:
 protected:
  void renderImpl() override;
};

}
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIIMAGE_H
