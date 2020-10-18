//
// Created by petr on 9/28/20.
//

#include "RenderPass.h"
#include "builders/RenderPassBuilder.h"
#include <utility>

namespace pf::vulkan {
RenderPass::RenderPass(RenderPassBuilder &builder, std::shared_ptr<LogicalDevice> device)
    : logicalDevice(std::move(device)) {
  auto [subpassNames, uniqueLogicalDevice] = builder.build(*logicalDevice);
  RenderPass::subPassNames = subpassNames;
  vkRenderPass = std::move(uniqueLogicalDevice);
}

const vk::RenderPass &RenderPass::getRenderPass() const { return vkRenderPass.get(); }

std::string RenderPass::info() const { return "Vulkan render pass unique"; }

const vk::RenderPass &RenderPass::operator*() const { return *vkRenderPass; }

vk::RenderPass const *RenderPass::operator->() const { return &*vkRenderPass; }

}// namespace pf::vulkan