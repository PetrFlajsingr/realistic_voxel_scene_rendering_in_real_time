//
// Created by petr on 10/28/20.
//

#include "Semaphore.h"
#include "LogicalDevice.h"

namespace pf::vulkan {

Semaphore::Semaphore(std::shared_ptr<LogicalDevice> device) : logicalDevice(std::move(device)) {
  auto createInfo = vk::SemaphoreCreateInfo();
  vkSemaphore = logicalDevice->getVkLogicalDevice().createSemaphoreUnique(createInfo);
}

const vk::Semaphore &Semaphore::operator*() const { return *vkSemaphore; }

vk::Semaphore const *Semaphore::operator->() const { return &*vkSemaphore; }

const vk::Semaphore &Semaphore::getVkSemaphore() const { return *vkSemaphore; }

LogicalDevice &Semaphore::getLogicalDevice() const {
  return *logicalDevice;
}
std::string Semaphore::info() const { return "Vulkan semaphore"; }
}// namespace pf::vulkan