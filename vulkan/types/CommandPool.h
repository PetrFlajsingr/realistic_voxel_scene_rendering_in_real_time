//
// Created by petr on 9/28/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_COMMANDPOOL_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_COMMANDPOOL_H

#include "../concepts/PtrConstructable.h"
#include "CommandBuffer.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

struct CommandSubmitConfig {
  std::vector<std::reference_wrapper<CommandBuffer>> commandBuffers;
  std::vector<vk::Semaphore> waitSemaphores;
  std::vector<vk::Semaphore> signalSemaphores;
  bool wait;
};

struct CommandPoolConfig {
  vk::QueueFlagBits queueFamily;
  vk::CommandPoolCreateFlagBits flags;
  vk::Queue queue;
};

class CommandPool : public VulkanObject, public PtrConstructable<CommandPool> {
 public:
  explicit CommandPool(std::shared_ptr<LogicalDevice> device, CommandPoolConfig &&config);

  CommandPool(const CommandPool &other) = delete;
  CommandPool &operator=(const CommandPool &other) = delete;

  [[nodiscard]] std::vector<std::shared_ptr<CommandBuffer>>
  createCommandBuffers(const CommandBufferConfig &config);

  void submitCommandBuffers(const CommandSubmitConfig &config);

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

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_COMMANDPOOL_H
