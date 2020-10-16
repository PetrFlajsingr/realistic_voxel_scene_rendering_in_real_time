//
// Created by petr on 9/27/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SURFACE_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SURFACE_H

#include "../concepts/PtrConstructable.h"
#include "../concepts/Window.h"
#include "fwd.h"
#include "VulkanObject.h"

namespace pf::vulkan {

template<window::Window Window>
struct SurfaceConfig {
  Instance &instance;
  Window &window;
};

class Surface : public VulkanObject, public PtrConstructable<Surface> {
 public:
  template<window::Window Window>
  explicit Surface(SurfaceConfig<Window> config)
      : vkSurface(config.window.createVulkanSurface(config.instance.getInstance())) {}

  Surface(const Surface &other) = delete;
  Surface &operator=(const Surface &other) = delete;

  [[nodiscard]] const vk::SurfaceKHR &getSurface();

  const vk::SurfaceKHR &operator*() const;
  vk::SurfaceKHR const *operator->() const;

  [[nodiscard]] std::string info() const override;

 private:
  vk::UniqueSurfaceKHR vkSurface;
};

}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SURFACE_H
