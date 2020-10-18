//
// Created by petr on 9/28/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_FRAMEBUFFER_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_FRAMEBUFFER_H

#include "../concepts/PtrConstructable.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

struct FrameBufferConfig {
  uint32_t width;
  uint32_t height;
  uint32_t layers;
};

namespace details {
class FrameBufferInstance : public VulkanObject {
 public:
  explicit FrameBufferInstance(FrameBuffer &parent, RenderPass &renderPass, SwapChain &swapChain,
                               uint32_t width, uint32_t height, uint32_t layers);
  [[nodiscard]] std::string info() const override;
  FrameBufferInstance(const FrameBufferInstance &other) = delete;
  FrameBufferInstance &operator=(const FrameBufferInstance &other) = delete;
  FrameBufferInstance(FrameBufferInstance &&other) = default;
  FrameBufferInstance &operator=(FrameBufferInstance &&other) = default;

  [[nodiscard]] const vk::Framebuffer &getFrameBuffer() const;

  const vk::Framebuffer &operator*() const;
  vk::Framebuffer const *operator->() const;

 private:
  vk::UniqueFramebuffer vkFrameBuffer;
  std::reference_wrapper<FrameBuffer> owner;
};
}// namespace details

class FrameBuffer : public VulkanObject, public PtrConstructable<FrameBuffer> {
 public:
  explicit FrameBuffer(std::shared_ptr<SwapChain> swap, FrameBufferConfig &&config);

  FrameBuffer(const FrameBuffer &other) = delete;
  FrameBuffer &operator=(const FrameBuffer &other) = delete;

  details::FrameBufferInstance &get(RenderPass &renderPass);

  [[nodiscard]] vk::Extent2D getExtent() const;
  [[nodiscard]] ImageView &getAttachment(uint32_t idx);

  [[nodiscard]] std::string info() const override;

 private:
  std::shared_ptr<SwapChain> swapChain;
  uint32_t width;
  uint32_t height;
  uint32_t layers;
  std::vector<std::shared_ptr<ImageView>> attachments;
  std::unordered_map<RenderPass *, details::FrameBufferInstance> instances;
};

}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_FRAMEBUFFER_H
