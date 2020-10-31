//
// Created by petr on 10/27/20.
//

#ifndef VOXEL_RENDER_IMGUIBASE_H
#define VOXEL_RENDER_IMGUIBASE_H

#include "../vulkan/types/CommandBuffer.h"
#include "elements/interface/ImGuiContainer.h"
#include <imgui.h>
namespace pf::ui {

class ImGuiInterface : public ImGuiContainer {
 public:
  explicit ImGuiInterface(ImGuiConfigFlags flags);

  virtual void addToCommandBuffer(vulkan::CommandBufferRecording &recording) = 0;

  [[nodiscard]] ImGuiIO &getIo() const;

 private:
  static ImGuiIO &baseInit(ImGuiConfigFlags flags);
  ImGuiIO &io;
};

}
#endif//VOXEL_RENDER_IMGUIBASE_H
