//
// Created by petr on 9/28/20.
//

#include "GraphicsPipeline.h"

namespace pf::vulkan {
GraphicsPipeline::GraphicsPipeline(vk::UniquePipeline &&pipeline, vk::UniquePipelineLayout &&layout,
                                   std::shared_ptr<RenderPass> pass)
    : vkPipeline(std::move(pipeline)), vkLayout(std::move(layout)),
      renderPass(std::move(pass)) {}

std::string GraphicsPipeline::info() const { return "Vulkan graphics pipeline unique"; }

const vk::Pipeline &GraphicsPipeline::getVkPipeline() const { return *vkPipeline; }

const vk::Pipeline &GraphicsPipeline::operator*() const { return *vkPipeline; }

vk::Pipeline const *GraphicsPipeline::operator->() const { return &*vkPipeline; }

const vk::PipelineLayout &GraphicsPipeline::getVkPipelineLayout() const { return *vkLayout; }
RenderPass &GraphicsPipeline::getRenderPass() const { return *renderPass; }

}// namespace pf::vulkan