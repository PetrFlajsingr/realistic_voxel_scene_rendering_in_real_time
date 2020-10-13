//
// Created by petr on 9/28/20.
//

#include "CommandPool.h"

pf::vulkan::CommandPool::CommandPool(vk::UniqueCommandPool &&vkCommandPool)
    : vkCommandPool(std::move(vkCommandPool)) {}

const vk::CommandPool &pf::vulkan::CommandPool::getCommandPool() const {
  return vkCommandPool.get();
}
std::string pf::vulkan::CommandPool::info() const { return "Vulkan command pool unique"; }
const vk::CommandPool &pf::vulkan::CommandPool::operator*() const { return *vkCommandPool; }
vk::CommandPool const *pf::vulkan::CommandPool::operator->() const { return &*vkCommandPool; }
