//
// Created by petr on 10/27/20.
//

#ifndef VOXEL_RENDER_IMGUIBASE_H
#define VOXEL_RENDER_IMGUIBASE_H

#include <imgui.h>
#include "../vulkan/types/CommandBuffer.h"
namespace pf::ui {

class ImGuiBase {
 public:
  explicit ImGuiBase(ImGuiConfigFlags flags);
  virtual ~ImGuiBase() = default;

  virtual void render() = 0;

  virtual void addToCommandBuffer(vulkan::CommandBufferRecording &recording) = 0;

  [[nodiscard]] ImGuiIO &getIo() const;

 private:
  static ImGuiIO &baseInit(ImGuiConfigFlags flags);
  ImGuiIO &io;
};

}
#endif//VOXEL_RENDER_IMGUIBASE_H
