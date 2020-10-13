//
// Created by petr on 9/28/20.
//

#include "FrameBuffer.h"
pf::vulkan::FrameBuffer::FrameBuffer(vk::UniqueFramebuffer &&vkBuffer)
    : vkBuffer(std::move(vkBuffer)) {}
const vk::Framebuffer &pf::vulkan::FrameBuffer::getFrameBuffer() const { return vkBuffer.get(); }
std::string pf::vulkan::FrameBuffer::info() const { return "Vulkan frame buffer unique"; }
const vk::Framebuffer &pf::vulkan::FrameBuffer::operator*() const { return *vkBuffer; }
vk::Framebuffer const *pf::vulkan::FrameBuffer::operator->() const { return &*vkBuffer; }
