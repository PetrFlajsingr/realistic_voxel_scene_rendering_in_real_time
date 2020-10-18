//
// Created by petr on 9/28/20.
//

#include "GraphicsPipelineBuilder.h"

namespace pf::vulkan {

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::setVertBindingDesc(const vk::VertexInputBindingDescription &description) {
  bindingDescription = description;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setVertAttributeDesc(
    const vk::VertexInputAttributeDescription &description) {
  inputAttributeDescription = description;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::addShader(Shader &sh) {
  auto create_info = vk::PipelineShaderStageCreateInfo();
  create_info.setStage(sh.getVkType())
      .setModule(sh.getShaderModule())
      .setPName(sh.getName().c_str());
  shaderStages.emplace_back(std::move(create_info));
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setTopology(const vk::PrimitiveTopology &top) {
  primitiveTopology = top;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setPrimitiveRestart(bool enabled) {
  primitiveRestartEnable = enabled;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setViewport(const vk::Viewport &vp) {
  viewports.emplace_back(vp);
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setScissor(const vk::Rect2D &scis) {
  scissors.emplace_back(scis);
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setDepthClamp(bool enabled) {
  depthClampEnabled = enabled;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setRastDiscard(bool enabled) {
  rasterizerDiscardEnabled = enabled;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setPolygonMode(vk::PolygonMode mode) {
  polygonMode = mode;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setLineWidth(float width) {
  lineWidth = width;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setCullMode(const vk::CullModeFlags &mode) {
  cullMode = mode;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setFrontFace(vk::FrontFace face) {
  frontFace = face;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setDepthBias(bool enabled) {
  depthBiasEnabled = enabled;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setDepthBiasConstFactor(float factor) {
  depthBiasConstFactor = factor;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setDepthBiasClamp(float clamp) {
  depthBiasClamp = clamp;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setDepthBiasSlopeFactor(float factor) {
  depthBiasSlopeFactor = factor;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setMsSampleShading(bool enabled) {
  msSampleShadingEnabled = enabled;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setMsSamples(vk::SampleCountFlagBits count) {
  msSampleCount = count;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setMsMinSampleShading(float min) {
  msMinSampleShading = min;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setMsSampleMask(vk::SampleMask mask) {
  msSampleMask = mask;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setMsAlphaToOne(bool enabled) {
  msAlphaToOne = enabled;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setBlend(bool enabled) {
  blendEnabled = enabled;
  return *this;
}
GraphicsPipelineBuilder &
GraphicsPipelineBuilder::setBlendColorMask(const vk::ColorComponentFlags &components) {
  blendColorMask = components;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setBlendSrcFactor(vk::BlendFactor factor) {
  blendSrcFactor = factor;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setBlendDstFactor(vk::BlendFactor factor) {
  blendDstFactor = factor;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setBlendColorOp(vk::BlendOp op) {
  blendColorOp = op;
  return *this;
}
GraphicsPipelineBuilder &
GraphicsPipelineBuilder::setBlendSrcAlphaBlendFactor(vk::BlendFactor factor) {
  blendSrcAlphaBlendFactor = factor;
  return *this;
}
GraphicsPipelineBuilder &
GraphicsPipelineBuilder::setBlendDstAlphaBlendFactor(vk::BlendFactor factor) {
  blendDstAlphaBlendFactor = factor;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setBlendAlphaBlendOp(vk::BlendOp op) {
  blendAlphaBlendOp = op;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setBlendLogicOpEnabled(bool enabled) {
  blendLogicOpEnabled = enabled;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setBlendLogicOp(vk::LogicOp op) {
  blendLogicOp = op;
  return *this;
}
GraphicsPipelineBuilder &
GraphicsPipelineBuilder::setBlendConstants(const std::array<float, 4> &constants) {
  blendConstants = constants;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setDynamicState(vk::DynamicState state) {
  dynamicState = state;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setDepthWrite(bool enabled) {
  depthWriteEnabled = enabled;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setDepthCompareOp(vk::CompareOp op) {
  depthCompareOp = op;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setDepthBoundTest(bool enabled) {
  depthBoundTestEnabled = enabled;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setDepthMin(float min) {
  depthMin = min;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setDepthMax(float max) {
  depthMax = max;
  return *this;
}
GraphicsPipelineBuilder &
GraphicsPipelineBuilder::setDepthFront(const std::vector<vk::StencilOpState> &states) {
  depthFrontStates = states;
  return *this;
}
GraphicsPipelineBuilder &
GraphicsPipelineBuilder::setDepthBack(const std::vector<vk::StencilOpState> &states) {
  depthBackStates = states;
  return *this;
}
GraphicsPipelineBuilder &GraphicsPipelineBuilder::setDescriptorSetLayouts(
    const std::vector<std::reference_wrapper<DescriptorSetLayout>> &layouts) {
  descriptorSetLayouts = layouts;
  return *this;
}
GraphicsPipelineBuilder &
GraphicsPipelineBuilder::setPushConstRange(const std::vector<vk::PushConstantRange> &range) {
  pushConstRange = range;
  return *this;
}

}// namespace pf::vulkan