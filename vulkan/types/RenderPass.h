//
// Created by petr on 9/28/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_RENDERPASS_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_RENDERPASS_H

#include "../concepts/PtrConstructable.h"
#include "fwd.h"
#include "VulkanObject.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {
class RenderPassBuilder;
class RenderPass : public VulkanObject, public PtrConstructable<RenderPass> {
 public:
  friend class RenderPassBuilder;
  RenderPass(RenderPassBuilder &builder, LogicalDevice &device);

  RenderPass(const RenderPass &other) = delete;
  RenderPass &operator=(const RenderPass &other) = delete;

  [[nodiscard]] const vk::RenderPass &getRenderPass() const;

  const vk::RenderPass &operator*() const;
  vk::RenderPass const *operator->() const;

  [[nodiscard]] std::string info() const override;

 private:
  vk::UniqueRenderPass vkRenderPass;
  std::vector<std::string> subPassNames;
};

}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_RENDERPASS_H
