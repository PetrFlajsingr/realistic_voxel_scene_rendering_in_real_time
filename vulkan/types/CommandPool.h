//
// Created by petr on 9/28/20.
//

#ifndef VOXEL_RENDER_COMMANDPOOL_H
#define VOXEL_RENDER_COMMANDPOOL_H

#include "../concepts/PtrConstructible.h"
#include "CommandBuffer.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

struct MultiCommandSubmitConfig {
  std::vector<std::reference_wrapper<CommandBuffer>> commandBuffers;
  std::vector<std::reference_wrapper<Semaphore>> waitSemaphores;
  std::vector<std::reference_wrapper<Semaphore>> signalSemaphores;
  vk::PipelineStageFlags flags;
  Fence &fence;
  bool wait;
};

struct CommandPoolConfig {
  vk::QueueFlagBits queueFamily;
  vk::CommandPoolCreateFlagBits flags;
};

class CommandPool : public VulkanObject,
                    public PtrConstructible<CommandPool>,
                    public std::enable_shared_from_this<CommandPool> {
 public:
  explicit CommandPool(std::shared_ptr<LogicalDevice> device, CommandPoolConfig &&config);

  CommandPool(const CommandPool &other) = delete;
  CommandPool &operator=(const CommandPool &other) = delete;

  [[nodiscard]] std::vector<std::shared_ptr<CommandBuffer>>
  createCommandBuffers(const CommandBufferConfig &config);

  void submitCommandBuffers(const MultiCommandSubmitConfig &config);

  [[nodiscard]] const vk::CommandPool &getCommandPool() const;

  const vk::CommandPool &operator*() const;
  vk::CommandPool const *operator->() const;

  [[nodiscard]] LogicalDevice &getDevice() const;

  [[nodiscard]] std::string info() const override;

 private:
  std::shared_ptr<LogicalDevice> logicalDevice;
  vk::UniqueCommandPool vkCommandPool;
  vk::Queue queue;
};

}// namespace pf::vulkan

#endif//VOXEL_RENDER_COMMANDPOOL_H
