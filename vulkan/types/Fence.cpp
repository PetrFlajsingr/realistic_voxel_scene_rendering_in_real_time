//
// Created by petr on 10/18/20.
//

#include "Fence.h"
#include "LogicalDevice.h"

namespace pf::vulkan {

Fence::Fence(std::shared_ptr<LogicalDevice> device, FenceConfig &&config)
    : logicalDevice(std::move(device)) {
  auto createInfo = vk::FenceCreateInfo();
  createInfo.flags = config.flags;
  vkFence = logicalDevice->getVkLogicalDevice().createFenceUnique(createInfo);
}

LogicalDevice &Fence::getLogicalDevice() const { return *logicalDevice; }

const vk::Fence &Fence::operator*() const { return *vkFence; }

vk::Fence const *Fence::operator->() const { return &*vkFence; }

const vk::Fence &Fence::getVkFence() const { return *vkFence; }
std::string Fence::info() const { return "Vulkan fence"; }

void Fence::wait() const {
  logicalDevice->getVkLogicalDevice().waitForFences(std::vector{*vkFence}, true,
                                                    std::numeric_limits<uint64_t>::max());
}

void Fence::reset() { logicalDevice->getVkLogicalDevice().resetFences({*vkFence}); }
}// namespace pf::vulkan