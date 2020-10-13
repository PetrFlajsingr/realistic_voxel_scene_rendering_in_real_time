//
// Created by petr on 9/28/20.
//

#include "GraphicsPipeline.h"

pf::vulkan::GraphicsPipeline::GraphicsPipeline(vk::UniquePipeline &&pipeline)
    : vkPipeline(std::move(pipeline)) {}
std::string pf::vulkan::GraphicsPipeline::info() const { return "Vulkan graphics pipeline unique"; }
const vk::Pipeline &pf::vulkan::GraphicsPipeline::getVkPipeline() const { return *vkPipeline; }
const vk::Pipeline &pf::vulkan::GraphicsPipeline::operator*() const { return *vkPipeline; }
vk::Pipeline const *pf::vulkan::GraphicsPipeline::operator->() const { return &*vkPipeline; }
