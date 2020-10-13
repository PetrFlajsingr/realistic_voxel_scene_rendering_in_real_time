//
// Created by petr on 9/28/20.
//

#include "CommandPool.h"

pf::vulkan::CommandPool::CommandPool(pf::vulkan::CommandPoolConfig &&config)
    : queue(config.device->getQueue(config.queueFamily)) {
  const auto queueFamilyIndex = config.device->getQueueIndices()[config.queueFamily];
  auto createInfo = vk::CommandPoolCreateInfo();
  createInfo.queueFamilyIndex = queueFamilyIndex;
  createInfo.flags = config.flags;
  vkCommandPool = config.device->getVkLogicalDevice().createCommandPoolUnique(createInfo);
  device = std::move(config.device);
}

const vk::CommandPool &pf::vulkan::CommandPool::getCommandPool() const {
  return vkCommandPool.get();
}

std::string pf::vulkan::CommandPool::info() const { return "Vulkan command pool unique"; }

const vk::CommandPool &pf::vulkan::CommandPool::operator*() const { return *vkCommandPool; }

vk::CommandPool const *pf::vulkan::CommandPool::operator->() const { return &*vkCommandPool; }

std::vector<std::shared_ptr<pf::vulkan::CommandBuffer>>
pf::vulkan::CommandPool::createCommandBuffers(const CommandBufferConfig &config) {
  auto allocateInfo = vk::CommandBufferAllocateInfo();
  allocateInfo.commandBufferCount = config.count;
  allocateInfo.level = config.level;
  allocateInfo.commandPool = *vkCommandPool;
  auto buffers = device->getVkLogicalDevice().allocateCommandBuffersUnique(allocateInfo);
  auto result = std::vector<std::shared_ptr<pf::vulkan::CommandBuffer>>();
  result.reserve(buffers.size());
  std::ranges::transform(buffers, std::back_inserter(result), [](auto &&buffer) {
    return CommandBuffer::CreateShared(std::move(buffer));
  });
  return result;
}

pf::vulkan::LogicalDevice &pf::vulkan::CommandPool::getDevice() const { return *device; }

void pf::vulkan::CommandPool::submitCommandBuffers(const CommandSubmitConfig &config) {
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

pf::vulkan::CommandBuffer::CommandBuffer(vk::UniqueCommandBuffer &&buffer)
    : vkBuffer(std::move(buffer)) {}

std::string pf::vulkan::CommandBuffer::info() const { return "Vulkan command buffer"; }

void pf::vulkan::CommandBuffer::begin(vk::CommandBufferUsageFlagBits flag) {
  auto beginInfo = vk::CommandBufferBeginInfo();
  beginInfo.flags = flag;
  vkBuffer->begin(beginInfo);
}

void pf::vulkan::CommandBuffer::end() { vkBuffer->end(); }

const vk::CommandBuffer &pf::vulkan::CommandBuffer::getVkBuffer() const { return *vkBuffer; }

const vk::CommandBuffer &pf::vulkan::CommandBuffer::operator*() const { return *vkBuffer; }

vk::CommandBuffer const *pf::vulkan::CommandBuffer::operator->() const { return &*vkBuffer; }
