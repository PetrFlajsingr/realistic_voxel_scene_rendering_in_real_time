//
// Created by petr on 10/18/20.
//

#include "CommandBuffer.h"

namespace pf::vulkan {

CommandBuffer::CommandBuffer(std::shared_ptr<CommandPool> pool, vk::UniqueCommandBuffer &&buffer)
    : vkBuffer(std::move(buffer)), commandPool(std::move(pool)) {}

std::string CommandBuffer::info() const { return "Vulkan command buffer"; }

void CommandBuffer::begin(vk::CommandBufferUsageFlagBits flag) {
  auto beginInfo = vk::CommandBufferBeginInfo();
  beginInfo.flags = flag;
  vkBuffer->begin(beginInfo);
}

void CommandBuffer::end() { vkBuffer->end(); }

const vk::CommandBuffer &CommandBuffer::getVkBuffer() const { return *vkBuffer; }

const vk::CommandBuffer &CommandBuffer::operator*() const { return *vkBuffer; }

vk::CommandBuffer const *CommandBuffer::operator->() const { return &*vkBuffer; }

CommandPool &CommandBuffer::getCommandPool() { return *commandPool; }

void CommandBuffer::reset() { vkBuffer.reset(); }
}// namespace pf::vulkan