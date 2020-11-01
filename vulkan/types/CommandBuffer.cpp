//
// Created by petr on 10/18/20.
//

#include "CommandBuffer.h"
#include "../VulkanException.h"
#include "Buffer.h"
#include "CommandPool.h"
#include "FrameBuffer.h"
#include "GraphicsPipeline.h"
#include "Image.h"
#include "RenderPass.h"
#include "SwapChain.h"

namespace pf::vulkan {

CommandBufferRecording::CommandBufferRecording(CommandBuffer &buffer) : owner(buffer) {
  owner.get().isRecording = true;
}

void CommandBufferRecording::end() {
  if (!isValid) { throw VulkanException("Command buffer recording has been ended already"); }
  isValid = false;
  owner.get()->end();
  owner.get().isRecording = false;
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
  owner.get()->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
  return *this;
}

CommandBufferRecording &CommandBufferRecording::endRenderPass() {
  owner.get()->endRenderPass();
  return *this;
}

CommandBufferRecording &CommandBufferRecording::bindPipeline(vk::PipelineBindPoint bindPoint,
                                                             GraphicsPipeline &pipeline) {
  owner.get()->bindPipeline(bindPoint, *pipeline);
  return *this;
}
CommandBufferRecording &CommandBufferRecording::draw(DrawCommand &&cmd) {
  owner.get()->draw(cmd.vertexCount, cmd.instanceCount, cmd.vertexOffset, cmd.instanceOffset);
  return *this;
}
CommandBuffer &CommandBufferRecording::getCommandBuffer() { return owner.get(); }
CommandBufferRecording::CommandBufferRecording(CommandBufferRecording &&other) noexcept
    : owner(other.owner) {
  other.isValid = false;
  isValid = true;
}
CommandBufferRecording &CommandBufferRecording::operator=(CommandBufferRecording &&other) noexcept {
  other.isValid = false;
  owner = other.owner;
  isValid = true;
  return *this;
}
CommandBufferRecording &CommandBufferRecording::copyBuffer(Buffer &src, Buffer &dst,
                                                           vk::DeviceSize srcOffset,
                                                           vk::DeviceSize dstOffset,
                                                           vk::DeviceSize range) {
  auto bufferCopy = vk::BufferCopy();
  bufferCopy.srcOffset = srcOffset;
  bufferCopy.dstOffset = dstOffset;
  bufferCopy.size = range;
  owner.get()->copyBuffer(*src, *dst, bufferCopy);
  return *this;
}

CommandBufferRecording &CommandBufferRecording::copyBufferToImage(
    Buffer &src, Image &dst, vk::DeviceSize srcOffset, uint32_t srcRowLength, uint32_t srcHeight,
    vk::Offset3D dstOffset, const vk::ImageSubresourceLayers& imageSubresourceLayers) {
  auto bufferCopy = vk::BufferImageCopy();
  bufferCopy.bufferOffset = 0;
  bufferCopy.bufferRowLength = 0;
  bufferCopy.bufferImageHeight = 0;
  bufferCopy.imageSubresource = imageSubresourceLayers;
  bufferCopy.imageOffset = dstOffset;
  bufferCopy.imageExtent = dst.getExtent();
  owner.get()->copyBufferToImage(*src, *dst, dst.getLayout(), 1, &bufferCopy);
  return *this;
}

CommandBufferRecording &
CommandBufferRecording::pipelineBarrier(vk::PipelineStageFlagBits srcStage,
                                        vk::PipelineStageFlagBits dstStage,
                                        const std::vector<vk::MemoryBarrier> &memoryBarrier,
                                        const std::vector<vk::BufferMemoryBarrier> &bufferBarrier,
                                        const std::vector<vk::ImageMemoryBarrier> &imageBarrier) {
  owner.get()->pipelineBarrier(srcStage, dstStage, {}, memoryBarrier, bufferBarrier, imageBarrier);
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

void CommandBuffer::submit(CommandSubmitConfig &&config) {
  commandPool->submitCommandBuffers({.commandBuffers = {*this},
                                     .waitSemaphores = std::move(config.waitSemaphores),
                                     .signalSemaphores = std::move(config.signalSemaphores),
                                     .flags = config.flags,
                                     .fence = config.fence,
                                     .wait = config.wait});
}
}// namespace pf::vulkan