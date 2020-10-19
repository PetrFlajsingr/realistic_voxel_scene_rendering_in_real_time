//
// Created by petr on 9/28/20.
//

#include "CommandPool.h"
#include "PhysicalDevice.h"

namespace pf::vulkan {
CommandPool::CommandPool(std::shared_ptr<LogicalDevice> device, CommandPoolConfig &&config)
    : logicalDevice(std::move(device)), queue(logicalDevice->getQueue(config.queueFamily)) {
  const auto queueFamilyIndex = logicalDevice->getQueueIndices()[config.queueFamily];
  auto createInfo = vk::CommandPoolCreateInfo();
  createInfo.queueFamilyIndex = queueFamilyIndex;
  createInfo.flags = config.flags;
  vkCommandPool = logicalDevice->getVkLogicalDevice().createCommandPoolUnique(createInfo);
}

const vk::CommandPool &CommandPool::getCommandPool() const { return vkCommandPool.get(); }

std::string CommandPool::info() const { return "Vulkan command pool unique"; }

const vk::CommandPool &CommandPool::operator*() const { return *vkCommandPool; }

vk::CommandPool const *CommandPool::operator->() const { return &*vkCommandPool; }

std::vector<std::shared_ptr<CommandBuffer>>
CommandPool::createCommandBuffers(const CommandBufferConfig &config) {
  auto allocateInfo = vk::CommandBufferAllocateInfo();
  allocateInfo.commandBufferCount = config.count;
  allocateInfo.level = config.level;
  allocateInfo.commandPool = *vkCommandPool;
  auto buffers = logicalDevice->getVkLogicalDevice().allocateCommandBuffersUnique(allocateInfo);
  auto result = std::vector<std::shared_ptr<CommandBuffer>>();
  result.reserve(buffers.size());
  std::ranges::transform(buffers, std::back_inserter(result), [this](auto &&buffer) {
    return CommandBuffer::CreateShared(shared_from_this(), std::move(buffer));
  });
  return result;
}

LogicalDevice &CommandPool::getDevice() const { return *logicalDevice; }

void CommandPool::submitCommandBuffers(const CommandSubmitConfig &config) {
  auto buffers = std::vector<vk::CommandBuffer>();
  buffers.reserve(config.commandBuffers.size());
  std::ranges::transform(config.commandBuffers, std::back_inserter(buffers),
                         [](const auto &buffer) { return *buffer.get(); });
  auto submitInfo = vk::SubmitInfo();
  submitInfo.setWaitSemaphores(config.waitSemaphores);
  submitInfo.setSignalSemaphores(config.signalSemaphores);
  submitInfo.setCommandBuffers(buffers);

  queue.submit({submitInfo}, vk::Fence());
  if (config.wait) { queue.waitIdle(); }
}

}// namespace pf::vulkan