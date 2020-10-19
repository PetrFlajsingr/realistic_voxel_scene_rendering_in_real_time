//
// Created by petr on 9/27/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SURFACE_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SURFACE_H

#include "../concepts/PtrConstructible.h"
#include "../concepts/Window.h"
#include "Instance.h"
#include "VulkanObject.h"
#include "fwd.h"

namespace pf::vulkan {



class Surface : public VulkanObject, public PtrConstructible<Surface> {
 public:
  template<window::Window Window>
  explicit Surface(std::shared_ptr<Instance> inst, Window &window)
      : instance(std::move(inst)), vkSurface(window.createVulkanSurface(**instance)) {}

  Surface(const Surface &other) = delete;
  Surface &operator=(const Surface &other) = delete;

  [[nodiscard]] const vk::SurfaceKHR &getSurface();
  [[nodiscard]] Instance &getInstance();

  const vk::SurfaceKHR &operator*() const;
  vk::SurfaceKHR const *operator->() const;

  [[nodiscard]] std::string info() const override;

 private:
  std::shared_ptr<Instance> instance;
  vk::UniqueSurfaceKHR vkSurface;
};

}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SURFACE_H
