//
// Created by petr on 10/27/20.
//

#include "DescriptorPool.h"
#include "LogicalDevice.h"

namespace pf::vulkan {

DescriptorPool::DescriptorPool(std::shared_ptr<LogicalDevice> device,
                               DescriptorPoolConfig &&config) : logicalDevice(std::move(device)) {
  auto createInfo = vk::DescriptorPoolCreateInfo();
  createInfo.flags = config.flags;
  createInfo.maxSets = config.maxSets;
  createInfo.setPoolSizes(config.poolSizes);
  vkDescriptorPool = logicalDevice->getVkLogicalDevice().createDescriptorPoolUnique(createInfo);
}

vk::DescriptorPool &DescriptorPool::getDescriptorPool() {
  return *vkDescriptorPool;
}
vk::DescriptorPool &DescriptorPool::operator*() {
  return *vkDescriptorPool;
}
vk::DescriptorPool *DescriptorPool::operator->() {
  return &*vkDescriptorPool;
}
LogicalDevice &DescriptorPool::getDevice() {
  return *logicalDevice;
}
}