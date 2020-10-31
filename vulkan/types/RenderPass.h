//
// Created by petr on 9/28/20.
//

#ifndef VOXEL_RENDER_RENDERPASS_H
#define VOXEL_RENDER_RENDERPASS_H

#include "../concepts/PtrConstructible.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {
class RenderPassBuilder;
class RenderPass : public VulkanObject, public PtrConstructible<RenderPass> {
 public:
  friend class RenderPassBuilder;
  RenderPass(RenderPassBuilder &builder, std::shared_ptr<LogicalDevice> device);

  RenderPass(const RenderPass &other) = delete;
  RenderPass &operator=(const RenderPass &other) = delete;

  [[nodiscard]] const vk::RenderPass &getRenderPass() const;
  [[nodiscard]] LogicalDevice &getLogicalDevice() const;

  const vk::RenderPass &operator*() const;
  vk::RenderPass const *operator->() const;

  [[nodiscard]] std::string info() const override;

 private:
  vk::UniqueRenderPass vkRenderPass;
  std::vector<std::string> subPassNames;
  std::shared_ptr<LogicalDevice> logicalDevice;
};

}// namespace pf::vulkan

#endif//VOXEL_RENDER_RENDERPASS_H
