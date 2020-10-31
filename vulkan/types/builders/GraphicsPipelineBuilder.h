//
// Created by petr on 9/28/20.
//

#ifndef VOXEL_RENDER_GRAPHICSPIPELINEBUILDER_H
#define VOXEL_RENDER_GRAPHICSPIPELINEBUILDER_H

#include "../../utils/common_enums.h"
#include "../DescriptorSetLayout.h"
#include "../Shader.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

namespace details {}

// TODO: dynamic state
// TODO: subbuilders for blends etc
class GraphicsPipelineBuilder {
 public:
  explicit GraphicsPipelineBuilder() = default;

  GraphicsPipelineBuilder &
  vertexInBindDescription(const vk::VertexInputBindingDescription &description);
  GraphicsPipelineBuilder &
  vertexInAttrDescription(const vk::VertexInputAttributeDescription &description);
  GraphicsPipelineBuilder &shader(Shader &sh, const std::string &name);
  GraphicsPipelineBuilder &topology(vk::PrimitiveTopology top);
  GraphicsPipelineBuilder &primitiveRestart(Enabled enabled);
  GraphicsPipelineBuilder &viewport(const vk::Viewport &vp);
  GraphicsPipelineBuilder &scissor(const vk::Rect2D &scis);
  GraphicsPipelineBuilder &depthClamp(Enabled enabled);
  GraphicsPipelineBuilder &rastDiscard(Enabled enabled);
  GraphicsPipelineBuilder &polygonMode(vk::PolygonMode mode);
  GraphicsPipelineBuilder &lineWidth(float width);
  GraphicsPipelineBuilder &cullMode(const vk::CullModeFlags &mode);
  GraphicsPipelineBuilder &frontFace(vk::FrontFace face);
  GraphicsPipelineBuilder &depthBias(Enabled enabled);
  GraphicsPipelineBuilder &depthBiasConstFactor(float factor);
  GraphicsPipelineBuilder &depthBiasClamp(float clamp);
  GraphicsPipelineBuilder &depthBiasSlopeFactor(float factor);
  GraphicsPipelineBuilder &setMsSampleShading(Enabled enabled);
  GraphicsPipelineBuilder &msSampleCount(vk::SampleCountFlagBits count);
  GraphicsPipelineBuilder &msMinSampleShading(float min);
  GraphicsPipelineBuilder &msSampleMask(vk::SampleMask mask);
  GraphicsPipelineBuilder &msAlphaToOne(Enabled enabled);
  // multiple blends
  GraphicsPipelineBuilder &blend(Enabled enabled);
  GraphicsPipelineBuilder &blendColorMask(const vk::ColorComponentFlags &components);
  GraphicsPipelineBuilder &blendSrcFactor(vk::BlendFactor factor);
  GraphicsPipelineBuilder &blendDstFactor(vk::BlendFactor factor);
  GraphicsPipelineBuilder &blendColorOp(vk::BlendOp op);
  GraphicsPipelineBuilder &blendSrcAlphaBlendFactor(vk::BlendFactor factor);
  GraphicsPipelineBuilder &blendDstAlphaBlendFactor(vk::BlendFactor factor);
  GraphicsPipelineBuilder &blendAlphaBlendOp(vk::BlendOp op);
  // -
  GraphicsPipelineBuilder &blendLogicOpEnabled(Enabled enabled);
  GraphicsPipelineBuilder &blendLogicOp(vk::LogicOp op);
  GraphicsPipelineBuilder &blendConstants(const std::array<float, 4> &constants);
  GraphicsPipelineBuilder &dynamicState(vk::DynamicState state);
  GraphicsPipelineBuilder &depthWrite(Enabled enabled);
  GraphicsPipelineBuilder &depthCompareOp(vk::CompareOp op);
  GraphicsPipelineBuilder &depthBoundTest(Enabled enabled);
  GraphicsPipelineBuilder &depthMin(float min);
  GraphicsPipelineBuilder &depthMax(float max);
  GraphicsPipelineBuilder &depthFront(const std::vector<vk::StencilOpState> &states);
  GraphicsPipelineBuilder &depthBack(const std::vector<vk::StencilOpState> &states);
  GraphicsPipelineBuilder &
  descriptorSetLayouts(const std::vector<std::reference_wrapper<DescriptorSetLayout>> &layouts);
  GraphicsPipelineBuilder &pushConstRange(const std::vector<vk::PushConstantRange> &range);
  GraphicsPipelineBuilder &rasterizationSamples(vk::SampleCountFlagBits samples);

  [[nodiscard]] std::shared_ptr<GraphicsPipeline> build(std::shared_ptr<RenderPass> renderPass);

 private:
  std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
  std::vector<vk::VertexInputAttributeDescription> inputAttributeDescriptions;
  std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
  vk::PrimitiveTopology primitiveTopology;
  bool primitiveRestartEnable{};
  std::vector<vk::Viewport> viewports;
  std::vector<vk::Rect2D> scissors;

  bool depthClampEnabled{};
  bool rasterizerDiscardEnabled{};
  vk::PolygonMode polygonMode_;
  float lineWidth_{};
  vk::CullModeFlags cullMode_;
  vk::FrontFace frontFace_;
  bool depthBiasEnabled{};
  float depthBiasConstFactor_{};
  float depthBiasClamp_{};
  float depthBiasSlopeFactor_{};
  bool msSampleShadingEnabled{};
  vk::SampleCountFlagBits msSampleCount_;
  float msMinSampleShading_{};
  vk::SampleMask msSampleMask_{};
  bool msAlphaToOneEnabled{};
  bool blendEnabled{};
  vk::ColorComponentFlags blendColorMask_;
  vk::BlendFactor blendSrcFactor_;
  vk::BlendFactor blendDstFactor_;
  vk::BlendOp blendColorOp_;
  vk::BlendFactor blendSrcAlphaBlendFactor_;
  vk::BlendFactor blendDstAlphaBlendFactor_;
  vk::BlendOp blendAlphaBlendOp_;
  bool blendLogicOpEnabled_{};
  vk::LogicOp blendLogicOp_;
  std::array<float, 4> blendConstants_{};
  vk::DynamicState dynamicState_;
  bool depthWriteEnabled{};
  vk::CompareOp depthCompareOp_;
  bool depthBoundTestEnabled{};
  float depthMin_{};
  float depthMax_{};
  std::vector<vk::StencilOpState> depthFrontStates;
  std::vector<vk::StencilOpState> depthBackStates;
  std::vector<std::reference_wrapper<DescriptorSetLayout>> descriptorSetLayouts_;
  std::vector<vk::PushConstantRange> pushConstRange_;
  vk::SampleCountFlagBits rasterizationSamples_;
};

}// namespace pf::vulkan
#endif//VOXEL_RENDER_GRAPHICSPIPELINEBUILDER_H
