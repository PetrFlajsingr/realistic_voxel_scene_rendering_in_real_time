//
// Created by petr on 12/5/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_RENDERING_RTTRIANGLERENDERER_H
#define REALISTIC_VOXEL_RENDERING_SRC_RENDERING_RTTRIANGLERENDERER_H

#include <pf_glfw_vulkan/concepts/Window.h>
#include <pf_glfw_vulkan/lib_config.h>
#include <pf_glfw_vulkan/vulkan/types.h>
#include <toml++/toml.h>
#include <utils/Camera.h>

using namespace pf::vulkan::literals;

namespace pf {
class RTTriangleRenderer {
 public:
  explicit RTTriangleRenderer(toml::table &tomlConfig);
  RTTriangleRenderer(const RTTriangleRenderer&) = delete;
  RTTriangleRenderer& operator=(const RTTriangleRenderer&) = delete;
  RTTriangleRenderer(RTTriangleRenderer&&) = default;
  RTTriangleRenderer& operator=(RTTriangleRenderer&&) = default;

  template<pf::ui::Window Window>
  void init([[maybe_unused]] Window &window) {
    throw NotImplementedException("init not implemented");
  }

  void render();

 private:
  std::reference_wrapper<toml::table> config;
  Camera camera;
};


}

#endif//REALISTIC_VOXEL_RENDERING_SRC_RENDERING_RTTRIANGLERENDERER_H
