//
// Created by petr on 9/28/20.
//

#include "FrameBuffer.h"
#include "ImageView.h"
#include "LogicalDevice.h"
#include "RenderPass.h"
#include "SwapChain.h"
#include <range/v3/view.hpp>

namespace pf::vulkan {
using namespace ranges;

const vk::Framebuffer &details::FrameBufferInstance::getFrameBuffer() const {
  return vkFrameBuffer.get();
}

const vk::Framebuffer &details::FrameBufferInstance::operator*() const { return *vkFrameBuffer; }

vk::Framebuffer const *details::FrameBufferInstance::operator->() const { return &*vkFrameBuffer; }

details::FrameBufferInstance::FrameBufferInstance(FrameBuffer &parent, RenderPass &renderPass,
                                                  SwapChain &swapChain, uint32_t width,
                                                  uint32_t height, uint32_t layers)
    : owner(parent) {
  auto createInfo = vk::FramebufferCreateInfo();
  createInfo.renderPass = *renderPass;
  auto attachments = swapChain.getImageViews()
      | views::transform([](auto &imageView) { return **imageView; }) | to_vector;
  createInfo.setAttachments(attachments);
  createInfo.width = width;
  createInfo.height = height;
  createInfo.layers = layers;
  vkFrameBuffer = swapChain.getLogicalDevice()->createFramebufferUnique(createInfo);
}

std::string details::FrameBufferInstance::info() const { return "Frame buffer accessor"; }

FrameBuffer::FrameBuffer(std::shared_ptr<SwapChain> swap)
    : swapChain(std::move(swap)), width(swapChain->getExtent().width),
      height(swapChain->getExtent().height), layers(1), attachments(swapChain->getImageViews()) {}

std::string FrameBuffer::info() const { return "Vulkan frame buffer unique"; }

vk::Extent2D FrameBuffer::getExtent() const { return {width, height}; }

details::FrameBufferInstance &FrameBuffer::get(RenderPass &renderPass) {
  if (auto iter = instances.find(&renderPass); iter != instances.end()) { return iter->second; }
  return instances
      .try_emplace(
          &renderPass,
          details::FrameBufferInstance(*this, renderPass, *swapChain, width, height, layers))
      .first->second;
}

ImageView &FrameBuffer::getAttachment(uint32_t idx) { return *attachments[idx]; }
}// namespace pf::vulkan