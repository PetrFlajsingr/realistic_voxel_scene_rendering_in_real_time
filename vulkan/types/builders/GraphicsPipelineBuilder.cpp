//
// Created by petr on 9/28/20.
//

#include "GraphicsPipelineBuilder.h"
#include "../GraphicsPipeline.h"
#include "../LogicalDevice.h"
#include "../RenderPass.h"

namespace pf::vulkan {
GraphicsPipelineBuilder &GraphicsPipelineBuilder::vertexInBindDescription(
    const vk::VertexInputBindingDescription &description) {
  bindingDescriptions.emplace_back(description);
  return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::vertexInAttrDescription(
    const vk::VertexInputAttributeDescription &description) {
  inputAttributeDescriptions.emplace_back(description);
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::shader(Shader &sh, const std::string &name) {
  auto createInfo = vk::PipelineShaderStageCreateInfo();
  createInfo.setStage(sh.getVkType())
      .setModule(sh.getShaderModule())
      .setPName(name.c_str());
  shaderStages.emplace_back(std::move(createInfo));
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::topology(vk::PrimitiveTopology top) {
  primitiveTopology = top;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::primitiveRestart(Enabled enabled) {
  primitiveRestartEnable = enabled == Enabled::Yes;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::viewport(const vk::Viewport &vp) {
  viewports.emplace_back(vp);
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::scissor(const vk::Rect2D &scis) {
  scissors.emplace_back(scis);
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::depthClamp(Enabled enabled) {
  depthClampEnabled = enabled == Enabled::Yes;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::rastDiscard(Enabled enabled) {
  rasterizerDiscardEnabled = enabled == Enabled::Yes;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::polygonMode(vk::PolygonMode mode) {
  polygonMode_ = mode;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::lineWidth(float width) {
  assert(width > 0);
  lineWidth_ = width;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::cullMode(const vk::CullModeFlags &mode) {
  cullMode_ = mode;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::frontFace(vk::FrontFace face) {
  frontFace_ = face;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::depthBias(Enabled enabled) {
  depthBiasEnabled = enabled == Enabled::Yes;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::depthBiasConstFactor(float factor) {
  depthBiasConstFactor_ = factor;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::depthBiasClamp(float clamp) {
  depthBiasClamp_ = clamp;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::depthBiasSlopeFactor(float factor) {
  depthBiasSlopeFactor_ = factor;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setMsSampleShading(Enabled enabled) {
  msSampleShadingEnabled = enabled == Enabled::Yes;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::msSampleCount(vk::SampleCountFlagBits count) {
  msSampleCount_ = count;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::msMinSampleShading(float min) {
  msMinSampleShading_ = min;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::msSampleMask(vk::SampleMask mask) {
  msSampleMask_ = mask;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::msAlphaToOne(Enabled enabled) {
  msAlphaToOneEnabled = enabled == Enabled::Yes;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::blend(Enabled enabled) {
  blendEnabled = enabled == Enabled::Yes;
  return *this;
}
GraphicsPipelineBuilder &
GraphicsPipelineBuilder::blendColorMask(const vk::ColorComponentFlags &components) {
  blendColorMask_ = components;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::blendSrcFactor(vk::BlendFactor factor) {
  blendSrcFactor_ = factor;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::blendDstFactor(vk::BlendFactor factor) {
  blendDstFactor_ = factor;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::blendColorOp(vk::BlendOp op) {
  blendColorOp_ = op;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::blendSrcAlphaBlendFactor(vk::BlendFactor factor) {
  blendSrcAlphaBlendFactor_ = factor;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::blendDstAlphaBlendFactor(vk::BlendFactor factor) {
  blendDstAlphaBlendFactor_ = factor;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::blendAlphaBlendOp(vk::BlendOp op) {
  blendAlphaBlendOp_ = op;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::blendLogicOpEnabled(Enabled enabled) {
  blendLogicOpEnabled_ = enabled == Enabled::Yes;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::blendLogicOp(vk::LogicOp op) {
  blendLogicOp_ = op;
  return *this;
}
GraphicsPipelineBuilder &
GraphicsPipelineBuilder::blendConstants(const std::array<float, 4> &constants) {
  blendConstants_ = constants;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::dynamicState(vk::DynamicState state) {
  dynamicState_ = state;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::depthWrite(Enabled enabled) {
  depthWriteEnabled = enabled == Enabled::Yes;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::depthCompareOp(vk::CompareOp op) {
  depthCompareOp_ = op;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::depthBoundTest(Enabled enabled) {
  depthBoundTestEnabled = enabled == Enabled::Yes;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::depthMin(float min) {
  depthMin_ = min;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::depthMax(float max) {
  depthMax_ = max;
  return *this;
}
GraphicsPipelineBuilder &
GraphicsPipelineBuilder::depthFront(const std::vector<vk::StencilOpState> &states) {
  depthFrontStates = states;
  return *this;
}
GraphicsPipelineBuilder &
GraphicsPipelineBuilder::depthBack(const std::vector<vk::StencilOpState> &states) {
  depthBackStates = states;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::descriptorSetLayouts(
    const std::vector<std::reference_wrapper<DescriptorSetLayout>> &layouts) {
  descriptorSetLayouts_ = layouts;
  return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::pushConstRange(const std::vector<vk::PushConstantRange> &range) {
  pushConstRange_ = range;
  return *this;
}
GraphicsPipelineBuilder &
GraphicsPipelineBuilder::rasterizationSamples(vk::SampleCountFlagBits samples) {
  rasterizationSamples_ = samples;
  return *this;
}
std::shared_ptr<GraphicsPipeline>
GraphicsPipelineBuilder::build(std::shared_ptr<RenderPass> renderPass) {
  auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo();
  vertexInputStateCreateInfo.setVertexAttributeDescriptions(inputAttributeDescriptions);
  vertexInputStateCreateInfo.setVertexBindingDescriptions(bindingDescriptions);

  auto vertexAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo();
  vertexAssemblyStateCreateInfo.topology = primitiveTopology;
  vertexAssemblyStateCreateInfo.primitiveRestartEnable = primitiveRestartEnable;

  auto viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo();
  viewportStateCreateInfo.setViewports(viewports);
  viewportStateCreateInfo.setScissors(scissors);

  auto rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo();
  rasterizationStateCreateInfo.depthClampEnable = depthClampEnabled;
  rasterizationStateCreateInfo.rasterizerDiscardEnable = rasterizerDiscardEnabled;
  rasterizationStateCreateInfo.polygonMode = polygonMode_;
  rasterizationStateCreateInfo.lineWidth = lineWidth_;
  rasterizationStateCreateInfo.cullMode = cullMode_;
  rasterizationStateCreateInfo.frontFace = frontFace_;
  rasterizationStateCreateInfo.depthBiasEnable = depthBiasEnabled;
  rasterizationStateCreateInfo.depthBiasClamp = depthBiasClamp_;
  rasterizationStateCreateInfo.depthBiasConstantFactor = depthBiasConstFactor_;
  rasterizationStateCreateInfo.depthBiasSlopeFactor = depthBiasSlopeFactor_;

  auto multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo();
  // TODO multisampleStateCreateInfo.alphaToCoverageEnable =
  multisampleStateCreateInfo.alphaToOneEnable = msAlphaToOneEnabled;
  multisampleStateCreateInfo.sampleShadingEnable = msSampleShadingEnabled;
  multisampleStateCreateInfo.minSampleShading = msMinSampleShading_;
  multisampleStateCreateInfo.rasterizationSamples = rasterizationSamples_;

  // TODO: multiple
  auto colorBlendAttState = vk::PipelineColorBlendAttachmentState();
  colorBlendAttState.blendEnable = blendEnabled;
  colorBlendAttState.alphaBlendOp = blendAlphaBlendOp_;
  colorBlendAttState.colorBlendOp = blendColorOp_;
  colorBlendAttState.colorWriteMask = blendColorMask_;
  colorBlendAttState.srcAlphaBlendFactor = blendSrcAlphaBlendFactor_;
  colorBlendAttState.dstAlphaBlendFactor = blendDstAlphaBlendFactor_;
  auto colorBlendAttachments = std::vector{colorBlendAttState};

  auto colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo();
  colorBlendStateCreateInfo.logicOpEnable = blendLogicOpEnabled_;
  colorBlendStateCreateInfo.logicOp = blendLogicOp_;
  colorBlendStateCreateInfo.setAttachments(colorBlendAttachments);
  colorBlendStateCreateInfo.setBlendConstants(blendConstants_);

  // TODO
  auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo();
  pipelineLayoutCreateInfo.setSetLayouts({});
  pipelineLayoutCreateInfo.setPushConstantRanges({});

  auto pipelineLayout =
      renderPass->getLogicalDevice()->createPipelineLayoutUnique(pipelineLayoutCreateInfo);

  auto graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo();
  graphicsPipelineCreateInfo.setStages(shaderStages);
  graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
  graphicsPipelineCreateInfo.pInputAssemblyState = &vertexAssemblyStateCreateInfo;
  graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
  graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
  graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
  graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
  graphicsPipelineCreateInfo.layout = *pipelineLayout;
  graphicsPipelineCreateInfo.renderPass = **renderPass;
  graphicsPipelineCreateInfo.subpass = 0;

  // TODO: cache
  //auto pipelineCacheCreateInfo = vk::PipelineCacheCreateInfo();
  //auto cache = renderPass->getLogicalDevice()->createPipelineCacheUnique(pipelineCacheCreateInfo);
  auto graphicsPipeline = renderPass->getLogicalDevice()->createGraphicsPipelineUnique(
      nullptr, graphicsPipelineCreateInfo);
  return GraphicsPipeline::CreateShared(std::move(graphicsPipeline), std::move(pipelineLayout),
                                        std::move(renderPass));
}

}// namespace pf::vulkan