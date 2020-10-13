//
// Created by petr on 9/28/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_GRAPHICSPIPELINEBUILDER_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_GRAPHICSPIPELINEBUILDER_H

#include "../DescriptorSetLayout.h"
#include "../Shader.h"
#include <vulkan/vulkan.hpp>
namespace pf::vulkan {

namespace details {}

// TODO: dynamic state
class GraphicsPipelineBuilder {
 public:
  GraphicsPipelineBuilder &setVertBindingDesc(const vk::VertexInputBindingDescription &description);
  GraphicsPipelineBuilder &
  setVertAttributeDesc(const vk::VertexInputAttributeDescription &description);
  GraphicsPipelineBuilder &addShader(Shader &sh);
  GraphicsPipelineBuilder &setTopology(vk::PrimitiveTopology &top);
  GraphicsPipelineBuilder &setPrimitiveRestart(bool enabled);
  GraphicsPipelineBuilder &setViewport(const vk::Viewport &vp);
  GraphicsPipelineBuilder &setScissor(const vk::Rect2D &scis);
  GraphicsPipelineBuilder &setDepthClamp(bool enabled);
  GraphicsPipelineBuilder &setRastDiscard(bool enabled);
  GraphicsPipelineBuilder &setPolygonMode(vk::PolygonMode mode);
  GraphicsPipelineBuilder &setLineWidth(float width);
  GraphicsPipelineBuilder &setCullMode(const vk::CullModeFlags &mode);
  GraphicsPipelineBuilder &setFrontFace(vk::FrontFace face);
  GraphicsPipelineBuilder &setDepthBias(bool enabled);
  GraphicsPipelineBuilder &setDepthBiasConstFactor(float factor);
  GraphicsPipelineBuilder &setDepthBiasClamp(float clamp);
  GraphicsPipelineBuilder &setDepthBiasSlopeFactor(float factor);
  GraphicsPipelineBuilder &setMsSampleShading(bool enabled);
  GraphicsPipelineBuilder &setMsSamples(vk::SampleCountFlagBits count);
  GraphicsPipelineBuilder &setMsMinSampleShading(float min);
  GraphicsPipelineBuilder &setMsSampleMask(vk::SampleMask mask);
  GraphicsPipelineBuilder &setMsAlphaToOne(bool enabled);
  // multiple blends
  GraphicsPipelineBuilder &setBlend(bool enabled);
  GraphicsPipelineBuilder &setBlendColorMask(const vk::ColorComponentFlags &components);
  GraphicsPipelineBuilder &setBlendSrcFactor(vk::BlendFactor factor);
  GraphicsPipelineBuilder &setBlendDstFactor(vk::BlendFactor factor);
  GraphicsPipelineBuilder &setBlendColorOp(vk::BlendOp op);
  GraphicsPipelineBuilder &setBlendSrcAlphaBlendFactor(vk::BlendFactor factor);
  GraphicsPipelineBuilder &setBlendDstAlphaBlendFactor(vk::BlendFactor factor);
  GraphicsPipelineBuilder &setBlendAlphaBlendOp(vk::BlendOp op);
  // -
  GraphicsPipelineBuilder &setBlendLogicOpEnabled(bool enabled);
  GraphicsPipelineBuilder &setBlendLogicOp(vk::LogicOp op);
  GraphicsPipelineBuilder &setBlendConstants(const std::array<float, 4> &constants);
  GraphicsPipelineBuilder &setDynamicState(vk::DynamicState state);
  GraphicsPipelineBuilder &setDepthWrite(bool enabled);
  GraphicsPipelineBuilder &setDepthCompareOp(vk::CompareOp op);
  GraphicsPipelineBuilder &setDepthBoundTest(bool enabled);
  GraphicsPipelineBuilder &setDepthMin(float min);
  GraphicsPipelineBuilder &setDepthMax(float max);
  GraphicsPipelineBuilder &setDepthFront(const std::vector<vk::StencilOpState> &states);
  GraphicsPipelineBuilder &setDepthBack(const std::vector<vk::StencilOpState> &states);
  GraphicsPipelineBuilder &
  setDescriptorSetLayouts(const std::vector<std::reference_wrapper<DescriptorSetLayout>> &layouts);
  GraphicsPipelineBuilder &setPushConstRange(const std::vector<vk::PushConstantRange> &range);

 private:
  vk::VertexInputBindingDescription bindingDescription;
  vk::VertexInputAttributeDescription inputAttributeDescription;
  std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
  vk::PrimitiveTopology primitiveTopology;
  bool primitiveRestartEnable;
  std::vector<vk::Viewport> viewports;
  std::vector<vk::Rect2D> scissors;

  bool depthClampEnabled{};
  bool rasterizerDiscardEnabled{};
  vk::PolygonMode polygonMode;
  float lineWidth;
  vk::CullModeFlags cullMode;
  vk::FrontFace frontFace;
  bool depthBiasEnabled;
  float depthBiasConstFactor;
  float depthBiasClamp;
  float depthBiasSlopeFactor;
  bool msSampleShadingEnabled;
  vk::SampleCountFlagBits msSampleCount;
  float msMinSampleShading;
  vk::SampleMask msSampleMask;
  bool msAlphaToOne;
  bool blendEnabled;
  vk::ColorComponentFlags blendColorMask;
  vk::BlendFactor blendSrcFactor;
  vk::BlendFactor blendDstFactor;
  vk::BlendOp blendColorOp;
  vk::BlendFactor blendSrcAlphaBlendFactor;
  vk::BlendFactor blendDstAlphaBlendFactor;
  vk::BlendOp blendAlphaBlendOp;
  bool blendLogicOpEnabled;
  vk::LogicOp blendLogicOp;
  std::array<float, 4> blendConstants;
  vk::DynamicState dynamicState;
  bool depthWriteEnabled;
  vk::CompareOp depthCompareOp;
  bool depthBoundTestEnabled;
  float depthMin;
  float depthMax;
  std::vector<vk::StencilOpState> depthFrontStates;
  std::vector<vk::StencilOpState> depthBackStates;
  std::vector<std::reference_wrapper<DescriptorSetLayout>> descriptorSetLayouts;
  std::vector<vk::PushConstantRange> pushConstRange;
};

}// namespace pf::vulkan
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_GRAPHICSPIPELINEBUILDER_H
