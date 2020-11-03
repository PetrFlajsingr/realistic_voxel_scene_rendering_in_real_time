//
// Created by petr on 10/31/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMAGE_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMAGE_H

#include "interface/ResizableElement.h"

namespace pf::ui::ig {

class Image : public ResizableElement {
 public:
 protected:
  void renderImpl() override;
};

}
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMAGE_H
