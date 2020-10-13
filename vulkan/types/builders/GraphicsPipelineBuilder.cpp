//
// Created by petr on 9/28/20.
//

#include "GraphicsPipelineBuilder.h"
pf::vulkan::GraphicsPipelineBuilder &pf::vulkan::GraphicsPipelineBuilder::setVertBindingDesc(
    const vk::VertexInputBindingDescription &description) {
  bindingDescription = description;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &pf::vulkan::GraphicsPipelineBuilder::setVertAttributeDesc(
    const vk::VertexInputAttributeDescription &description) {
  inputAttributeDescription = description;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::addShader(pf::vulkan::Shader &sh) {
  auto create_info = vk::PipelineShaderStageCreateInfo();
  create_info.setStage(sh.getVkType())
      .setModule(sh.getShaderModule())
      .setPName(sh.getName().c_str());
  shaderStages.emplace_back(std::move(create_info));
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setTopology(vk::PrimitiveTopology &top) {
  primitiveTopology = top;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setPrimitiveRestart(bool enabled) {
  primitiveRestartEnable = enabled;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setViewport(const vk::Viewport &vp) {
  viewports.emplace_back(vp);
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setScissor(const vk::Rect2D &scis) {
  scissors.emplace_back(scis);
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setDepthClamp(bool enabled) {
  depthClampEnabled = enabled;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setRastDiscard(bool enabled) {
  rasterizerDiscardEnabled = enabled;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setPolygonMode(vk::PolygonMode mode) {
  polygonMode = mode;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setLineWidth(float width) {
  lineWidth = width;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setCullMode(const vk::CullModeFlags &mode) {
  cullMode = mode;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setFrontFace(vk::FrontFace face) {
  frontFace = face;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setDepthBias(bool enabled) {
  depthBiasEnabled = enabled;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setDepthBiasConstFactor(float factor) {
  depthBiasConstFactor = factor;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setDepthBiasClamp(float clamp) {
  depthBiasClamp = clamp;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setDepthBiasSlopeFactor(float factor) {
  depthBiasSlopeFactor = factor;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setMsSampleShading(bool enabled) {
  msSampleShadingEnabled = enabled;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setMsSamples(vk::SampleCountFlagBits count) {
  msSampleCount = count;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setMsMinSampleShading(float min) {
  msMinSampleShading = min;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setMsSampleMask(vk::SampleMask mask) {
  msSampleMask = mask;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setMsAlphaToOne(bool enabled) {
  msAlphaToOne = enabled;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &pf::vulkan::GraphicsPipelineBuilder::setBlend(bool enabled) {
  blendEnabled = enabled;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setBlendColorMask(const vk::ColorComponentFlags &components) {
  blendColorMask = components;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setBlendSrcFactor(vk::BlendFactor factor) {
  blendSrcFactor = factor;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setBlendDstFactor(vk::BlendFactor factor) {
  blendDstFactor = factor;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setBlendColorOp(vk::BlendOp op) {
  blendColorOp = op;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setBlendSrcAlphaBlendFactor(vk::BlendFactor factor) {
  blendSrcAlphaBlendFactor = factor;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setBlendDstAlphaBlendFactor(vk::BlendFactor factor) {
  blendDstAlphaBlendFactor = factor;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setBlendAlphaBlendOp(vk::BlendOp op) {
  blendAlphaBlendOp = op;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setBlendLogicOpEnabled(bool enabled) {
  blendLogicOpEnabled = enabled;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setBlendLogicOp(vk::LogicOp op) {
  blendLogicOp = op;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setBlendConstants(const std::array<float, 4> &constants) {
  blendConstants = constants;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setDynamicState(vk::DynamicState state) {
  dynamicState = state;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setDepthWrite(bool enabled) {
  depthWriteEnabled = enabled;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setDepthCompareOp(vk::CompareOp op) {
  depthCompareOp = op;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setDepthBoundTest(bool enabled) {
  depthBoundTestEnabled = enabled;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &pf::vulkan::GraphicsPipelineBuilder::setDepthMin(float min) {
  depthMin = min;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &pf::vulkan::GraphicsPipelineBuilder::setDepthMax(float max) {
  depthMax = max;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setDepthFront(const std::vector<vk::StencilOpState> &states) {
  depthFrontStates = states;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &
pf::vulkan::GraphicsPipelineBuilder::setDepthBack(const std::vector<vk::StencilOpState> &states) {
  depthBackStates = states;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &pf::vulkan::GraphicsPipelineBuilder::setDescriptorSetLayouts(
    const std::vector<std::reference_wrapper<DescriptorSetLayout>> &layouts) {
  descriptorSetLayouts = layouts;
  return *this;
}
pf::vulkan::GraphicsPipelineBuilder &pf::vulkan::GraphicsPipelineBuilder::setPushConstRange(
    const std::vector<vk::PushConstantRange> &range) {
  pushConstRange = range;
  return *this;
}
