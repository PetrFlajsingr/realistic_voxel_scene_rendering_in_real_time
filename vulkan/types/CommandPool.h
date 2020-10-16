//
// Created by petr on 9/28/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_COMMANDPOOL_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_COMMANDPOOL_H

#include "../concepts/PtrConstructable.h"
#include "fwd.h"
#include "VulkanObject.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

struct CommandPoolConfig {
  vk::QueueFlagBits queueFamily;
  vk::CommandPoolCreateFlagBits flags;
  std::shared_ptr<LogicalDevice> device;
  vk::Queue queue;
};

struct CommandBufferConfig {
  vk::CommandBufferLevel level;
  uint32_t count;
};

class CommandPool;
class CommandBuffer : public VulkanObject, public PtrConstructable<CommandBuffer> {
 public:
  explicit CommandBuffer(vk::UniqueCommandBuffer &&buffer);

  CommandBuffer(const CommandBuffer &other) = delete;
  CommandBuffer &operator=(const CommandBuffer &other) = delete;

  void begin(vk::CommandBufferUsageFlagBits flag);
  void end();

  [[nodiscard]] const vk::CommandBuffer &getVkBuffer() const;

  const vk::CommandBuffer &operator*() const;
  vk::CommandBuffer const *operator->() const;

  [[nodiscard]] std::string info() const override;

 private:
  vk::UniqueCommandBuffer vkBuffer;
};

struct CommandSubmitConfig {
  std::vector<std::reference_wrapper<CommandBuffer>> commandBuffers;
  std::vector<vk::Semaphore> waitSemaphores;
  std::vector<vk::Semaphore> signalSemaphores;
  bool wait;
};

class CommandPool : public VulkanObject, public PtrConstructable<CommandPool> {
 public:
  explicit CommandPool(CommandPoolConfig &&config);

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
  vk::UniqueCommandPool vkCommandPool;
  std::shared_ptr<LogicalDevice> device;
  vk::Queue queue;
};


}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_COMMANDPOOL_H
