//
// Created by petr on 10/18/20.
//

#ifndef VOXEL_RENDER_COMMANDBUFFER_H
#define VOXEL_RENDER_COMMANDBUFFER_H

#include "../concepts/PtrConstructible.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

struct CommandBufferConfig {
  vk::CommandBufferLevel level;
  uint32_t count;
};

struct ClearFrameBuffersCommand {
  RenderPass &renderPass;
  FrameBuffer &frameBuffer;
  std::vector<vk::ClearValue> clearValues;
  vk::Extent2D extent;
};

struct DrawCommand {
  uint32_t vertexCount;
  uint32_t instanceCount;
  uint32_t vertexOffset;
  uint32_t instanceOffset;
};

class CommandBufferRecording {
 public:
  explicit CommandBufferRecording(CommandBuffer &buffer);
  ~CommandBufferRecording();

  CommandBufferRecording & beginRenderPass(ClearFrameBuffersCommand &&cmd);
  CommandBufferRecording & endRenderPass();

  CommandBufferRecording & bindPipeline(vk::PipelineBindPoint bindPoint, GraphicsPipeline &pipeline);
  CommandBufferRecording & draw(DrawCommand &&cmd);


  void end();
 private:
  bool isValid = true;
  CommandBuffer &owner;
};

struct CommandSubmitConfig {
  std::vector<std::reference_wrapper<Semaphore>> waitSemaphores;
  std::vector<std::reference_wrapper<Semaphore>> signalSemaphores;
  vk::PipelineStageFlags flags;
  Fence &fence;
  bool wait;
};

class CommandBuffer : public VulkanObject, public PtrConstructible<CommandBuffer> {
 public:
  friend class CommandBufferRecording;
  explicit CommandBuffer(std::shared_ptr<CommandPool> pool, vk::UniqueCommandBuffer &&buffer);

  CommandBuffer(const CommandBuffer &other) = delete;
  CommandBuffer &operator=(const CommandBuffer &other) = delete;

  CommandBufferRecording begin(vk::CommandBufferUsageFlagBits flag);
  void reset();

  [[nodiscard]] CommandPool &getCommandPool();

  [[nodiscard]] const vk::CommandBuffer &getVkBuffer() const;

  const vk::CommandBuffer &operator*() const;
  vk::CommandBuffer const *operator->() const;

  [[nodiscard]] std::string info() const override;

  void submit(CommandSubmitConfig &&config);

 private:
  vk::UniqueCommandBuffer vkBuffer;
  std::shared_ptr<CommandPool> commandPool;
  bool isRecording = false;
};
}

#endif//VOXEL_RENDER_COMMANDBUFFER_H
