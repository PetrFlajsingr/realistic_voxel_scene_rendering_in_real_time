//
// Created by petr on 9/28/20.
//

#include "RenderPass.h"
#include "builders/RenderPassBuilder.h"
#include <utility>

pf::vulkan::RenderPass::RenderPass(pf::vulkan::RenderPassBuilder &builder, LogicalDevice &device) {
  auto [subpassNames, uniqueLogicalDevice] = builder.build(device);
  RenderPass::subPassNames = subpassNames;
  vkRenderPass = std::move(uniqueLogicalDevice);
}

const vk::RenderPass &pf::vulkan::RenderPass::getRenderPass() const { return vkRenderPass.get(); }

std::string pf::vulkan::RenderPass::info() const { return "Vulkan render pass unique"; }

const vk::RenderPass &pf::vulkan::RenderPass::operator*() const { return *vkRenderPass; }

vk::RenderPass const *pf::vulkan::RenderPass::operator->() const { return &*vkRenderPass; }
