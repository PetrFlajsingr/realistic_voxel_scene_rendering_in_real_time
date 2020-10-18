//
// Created by petr on 9/28/20.
//

#include "GraphicsPipeline.h"

namespace pf::vulkan {
GraphicsPipeline::GraphicsPipeline(vk::UniquePipeline &&pipeline)
    : vkPipeline(std::move(pipeline)) {}

std::string GraphicsPipeline::info() const { return "Vulkan graphics pipeline unique"; }

const vk::Pipeline &GraphicsPipeline::getVkPipeline() const { return *vkPipeline; }

const vk::Pipeline &GraphicsPipeline::operator*() const { return *vkPipeline; }

vk::Pipeline const *GraphicsPipeline::operator->() const { return &*vkPipeline; }

}// namespace pf::vulkan