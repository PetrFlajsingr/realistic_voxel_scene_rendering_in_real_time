//
// Created by petr on 10/18/20.
//

#include "CommandBuffer.h"
#include "../VulkanException.h"
#include "CommandPool.h"
#include "FrameBuffer.h"
#include "GraphicsPipeline.h"
#include "RenderPass.h"
#include "SwapChain.h"

namespace pf::vulkan {

CommandBufferRecording::CommandBufferRecording(CommandBuffer &buffer) : owner(buffer) {
  owner.isRecording = true;
}

void CommandBufferRecording::end() {
  if (!isValid) { throw VulkanException("Command buffer recording has been ended already"); }
  isValid = false;
  owner->end();
  owner.isRecording = false;
}

CommandBufferRecording::~CommandBufferRecording() {
  if (isValid) { end(); }
}

CommandBufferRecording &CommandBufferRecording::beginRenderPass(ClearFrameBuffersCommand &&cmd) {
  auto renderPassInfo = vk::RenderPassBeginInfo();
  renderPassInfo.renderPass = *cmd.renderPass;
  renderPassInfo.renderArea = vk::Rect2D{{0, 0}, cmd.extent};
  renderPassInfo.setClearValues(cmd.clearValues);
  renderPassInfo.framebuffer = *cmd.frameBuffer.get(cmd.renderPass);
  owner->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
  return *this;
}

CommandBufferRecording &CommandBufferRecording::endRenderPass() {
  owner->endRenderPass();
  return *this;
}

CommandBufferRecording &CommandBufferRecording::bindPipeline(vk::PipelineBindPoint bindPoint,
                                                             GraphicsPipeline &pipeline) {
  owner->bindPipeline(bindPoint, *pipeline);
  return *this;
}
CommandBufferRecording &CommandBufferRecording::draw(DrawCommand &&cmd) {
  owner->draw(cmd.vertexCount, cmd.instanceCount, cmd.vertexOffset, cmd.instanceOffset);
  return *this;
}

CommandBuffer::CommandBuffer(std::shared_ptr<CommandPool> pool, vk::UniqueCommandBuffer &&buffer)
    : vkBuffer(std::move(buffer)), commandPool(std::move(pool)) {}

std::string CommandBuffer::info() const { return "Vulkan command buffer"; }

CommandBufferRecording CommandBuffer::begin(vk::CommandBufferUsageFlagBits flag) {
  if (isRecording) { throw VulkanException("Command buffer is already recording"); }
  auto beginInfo = vk::CommandBufferBeginInfo();
  beginInfo.flags = flag;
  vkBuffer->begin(beginInfo);
  return CommandBufferRecording(*this);
}

const vk::CommandBuffer &CommandBuffer::getVkBuffer() const { return *vkBuffer; }

const vk::CommandBuffer &CommandBuffer::operator*() const { return *vkBuffer; }

vk::CommandBuffer const *CommandBuffer::operator->() const { return &*vkBuffer; }

CommandPool &CommandBuffer::getCommandPool() { return *commandPool; }
void CommandBuffer::reset() { vkBuffer.reset(); }

void CommandBuffer::submit(CommandSubmitConfig &&config) {
  commandPool->submitCommandBuffers({.commandBuffers = {*this},
                                     .waitSemaphores = std::move(config.waitSemaphores),
                                     .signalSemaphores = std::move(config.signalSemaphores),
                                     .flags = config.flags,
                                     .fence = config.fence,
                                     .wait = config.wait});
}
}// namespace pf::vulkan