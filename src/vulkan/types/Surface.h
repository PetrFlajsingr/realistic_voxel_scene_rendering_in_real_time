//
// Created by petr on 9/27/20.
//

#ifndef VOXEL_RENDER_SURFACE_H
#define VOXEL_RENDER_SURFACE_H

#include <pf_common/concepts/PtrConstructible.h>
#include "concepts/Window.h"
#include "Instance.h"
#include "VulkanObject.h"
#include "fwd.h"

namespace pf::vulkan {

class Surface : public VulkanObject, public PtrConstructible<Surface> {
 public:
  template<ui::Window Window>
  explicit Surface(std::shared_ptr<Instance> inst, Window &window)
      : instance(std::move(inst)), vkSurface(window.createVulkanSurface(**instance)),
        windowSizeFnc([&window] { return window.getResolution(); }) {}

  Surface(const Surface &other) = delete;
  Surface &operator=(const Surface &other) = delete;

  [[nodiscard]] const vk::SurfaceKHR &getSurface();
  [[nodiscard]] Instance &getInstance();

  const vk::SurfaceKHR &operator*() const;
  vk::SurfaceKHR const *operator->() const;

  [[nodiscard]] std::string info() const override;

  [[nodiscard]] ui::Resolution getWindowSize() const;

 private:
  std::shared_ptr<Instance> instance;
  vk::UniqueSurfaceKHR vkSurface;

  std::function<ui::Resolution()> windowSizeFnc;
};

}// namespace pf::vulkan

#endif//VOXEL_RENDER_SURFACE_H
