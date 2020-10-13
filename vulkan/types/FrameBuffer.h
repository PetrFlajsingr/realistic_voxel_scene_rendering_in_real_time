//
// Created by petr on 9/28/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_FRAMEBUFFER_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_FRAMEBUFFER_H

#include "../concepts/PtrConstructable.h"
#include "VulkanObject.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {
class FrameBuffer : public VulkanObject, public PtrConstructable<FrameBuffer> {
 public:
  // TODO: config construction
  explicit FrameBuffer(vk::UniqueFramebuffer &&vkBuffer);

  FrameBuffer(const FrameBuffer &other) = delete;
  FrameBuffer &operator=(const FrameBuffer &other) = delete;

  [[nodiscard]] const vk::Framebuffer &getFrameBuffer() const;
  const vk::Framebuffer &operator*() const;

  vk::Framebuffer const *operator->() const;

  [[nodiscard]] std::string info() const override;

 private:
  vk::UniqueFramebuffer vkBuffer;
};

}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_FRAMEBUFFER_H
