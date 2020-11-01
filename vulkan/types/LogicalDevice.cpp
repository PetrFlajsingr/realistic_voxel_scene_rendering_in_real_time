//
// Created by petr on 10/18/20.
//

#include "LogicalDevice.h"

namespace pf::vulkan {

LogicalDevice::LogicalDevice(const std::shared_ptr<PhysicalDevice> &device,
                             vk::UniqueDevice &&vkLogicalDevice,
                             std::unordered_map<vk::QueueFlagBits, uint32_t> queueIndices,
                             const std::optional<uint32_t> &presentQueueIndex)
    : physDevice(device), vkLogicalDevice(std::move(vkLogicalDevice)),
      queueIndices(std::move(queueIndices)), presentQueueIndex(presentQueueIndex) {}

const vk::Device &LogicalDevice::getVkLogicalDevice() const { return vkLogicalDevice.get(); }

std::unordered_map<vk::QueueFlagBits, uint32_t> &LogicalDevice::getQueueIndices() {
  return queueIndices;
}

const std::optional<uint32_t> &LogicalDevice::getPresentQueueIndex() const {
  return presentQueueIndex;
}

std::string LogicalDevice::info() const { return "Vulkan logical device unique"; }

const vk::Device &LogicalDevice::operator*() const { return *vkLogicalDevice; }

vk::Device const *LogicalDevice::operator->() const { return &*vkLogicalDevice; }

PhysicalDevice &LogicalDevice::getPhysicalDevice() const {
  auto ptr = physDevice.lock();
  return *ptr;
}

vk::Queue LogicalDevice::getQueue(vk::QueueFlagBits type) {
  return vkLogicalDevice->getQueue(queueIndices[type], 0);
}

std::shared_ptr<ImageUnique> LogicalDevice::createImage(ImageConfig &&config) {
  return ImageUnique::CreateShared(shared_from_this(), std::move(config));
}

std::shared_ptr<SwapChain> LogicalDevice::createSwapChain(std::shared_ptr<Surface> surface,
                                                          SwapChainConfig &&config) {
  auto result = SwapChain::CreateShared(std::move(surface), shared_from_this(), std::move(config));
  result->init();
  return result;
}

std::shared_ptr<CommandPool> LogicalDevice::createCommandPool(CommandPoolConfig &&config) {
  return CommandPool::CreateShared(shared_from_this(), std::move(config));
}

void LogicalDevice::wait() { vkLogicalDevice->waitIdle(); }

std::shared_ptr<DescriptorSetLayout>
LogicalDevice::createDescriptorSetLayout(DescriptorSetLayoutConfig &&config) {
  return DescriptorSetLayout::CreateShared(shared_from_this(), std::move(config));
}
std::shared_ptr<DescriptorPool> LogicalDevice::createDescriptorPool(DescriptorPoolConfig &&config) {
  return DescriptorPool::CreateShared(shared_from_this(), std::move(config));
}
std::shared_ptr<Fence> LogicalDevice::createFence(FenceConfig &&config) {
  return Fence::CreateShared(shared_from_this(), std::move(config));
}
std::shared_ptr<Semaphore> LogicalDevice::createSemaphore() {
  return Semaphore::CreateShared(shared_from_this());
}
vk::Queue LogicalDevice::getPresentQueue() {
  return vkLogicalDevice->getQueue(presentQueueIndex.value(), 0);
}
std::shared_ptr<Buffer> LogicalDevice::createBuffer(BufferConfig &&config, bool allocateImmediately = true) {
  return std::make_shared<Buffer>(shared_from_this(), std::move(config), allocateImmediately);
}

}// namespace pf::vulkan