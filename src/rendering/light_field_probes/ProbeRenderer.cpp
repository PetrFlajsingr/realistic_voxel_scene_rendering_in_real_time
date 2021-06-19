//
// Created by petr on 6/19/21.
//

#include "ProbeRenderer.h"

#include <utility>

namespace pf::lfp {

ProbeRenderer::ProbeRenderer(toml::table config, std::shared_ptr<vulkan::Instance> vkInstance,
                             std::shared_ptr<vulkan::PhysicalDevice> vkDevice,
                             std::shared_ptr<vulkan::LogicalDevice> vkLogicalDevice,
                             std::shared_ptr<vulkan::Buffer> svoBuffer, std::shared_ptr<vulkan::Buffer> modelInfoBuffer,
                             std::shared_ptr<vulkan::Buffer> bvhBuffer)
    : config(std::move(config)), vkInstance(std::move(vkInstance)), vkDevice(std::move(vkDevice)),
      vkLogicalDevice(std::move(vkLogicalDevice)), svoBuffer(std::move(svoBuffer)),
      modelInfoBuffer(std::move(modelInfoBuffer)), bvhBuffer(std::move(bvhBuffer)) {
  createDescriptorPool();
  createTextures();
  createPipeline();
  createCommands();
  createFences();
  recordCommands();
}

void ProbeRenderer::createDescriptorPool() {
  vkDescPool = vkLogicalDevice->createDescriptorPool({.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                                      .maxSets = 1,
                                                      .poolSizes = {
                                                          {vk::DescriptorType::eStorageBuffer, 1},// svo
                                                          {vk::DescriptorType::eUniformBuffer, 1},// debug
                                                          {vk::DescriptorType::eStorageBuffer, 1},// model infos
                                                          {vk::DescriptorType::eStorageBuffer, 1},// bvh
                                                          {vk::DescriptorType::eStorageBuffer, 1},// probes
                                                          {vk::DescriptorType::eStorageImage, 1}, // probe array
                                                          {vk::DescriptorType::eStorageImage, 1}, // probe debug
                                                      }});
}
void ProbeRenderer::createTextures() {
  using namespace vulkan;
  vkProbesDebugImage =
      vkLogicalDevice->createImage({.imageType = vk::ImageType::e2D,
                                    .format = vk::Format::eB8G8R8A8Unorm,
                                    .extent = vk::Extent3D{.width = 1024, .height = 1024, .depth = 1},
                                    .mipLevels = 1,
                                    .arrayLayers = 1,
                                    .sampleCount = vk::SampleCountFlagBits::e1,
                                    .tiling = vk::ImageTiling::eOptimal,
                                    .usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc
                                        | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                                    .sharingQueues = {},
                                    .layout = vk::ImageLayout::eUndefined});
  vkProbesDebugImageView =
      vkProbesDebugImage->createImageView(vk::ColorSpaceKHR::eSrgbNonlinear, vk::ImageViewType::e2D,
                                          vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
  vkProbesDebugImageSampler = TextureSampler::CreateShared(
      vkLogicalDevice,
      TextureSamplerConfig{.magFilter = vk::Filter::eLinear,
                           .minFilter = vk::Filter::eLinear,
                           .addressMode = {.u = vk::SamplerAddressMode::eClampToBorder,
                                           .v = vk::SamplerAddressMode::eClampToBorder,
                                           .w = vk::SamplerAddressMode::eClampToBorder},
                           .maxAnisotropy = std::nullopt,
                           .borderColor = vk::BorderColor::eFloatOpaqueBlack,
                           .unnormalizedCoordinates = false,
                           .compareOp = std::nullopt,
                           .mip = {.mode = vk::SamplerMipmapMode::eLinear, .lodBias = 0, .minLod = 0, .maxLod = 1}});

  vkProbesImage =
      vkLogicalDevice->createImage({.imageType = vk::ImageType::e2D,
                                    .format = vk::Format::eR32G32Sfloat,
                                    .extent = vk::Extent3D{.width = 1024, .height = 1024, .depth = 1},
                                    .mipLevels = 1,
                                    .arrayLayers = PROBES_IMG_ARRAY_LAYERS,
                                    .sampleCount = vk::SampleCountFlagBits::e1,
                                    .tiling = vk::ImageTiling::eOptimal,
                                    .usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc
                                        | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                                    .sharingQueues = {},
                                    .layout = vk::ImageLayout::eUndefined});
  vkProbesImageView = vkProbesImage->createImageView(
      vk::ColorSpaceKHR::eSrgbNonlinear, vk::ImageViewType::e2DArray,
      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, PROBES_IMG_ARRAY_LAYERS});
}
void ProbeRenderer::createPipeline() {
  using namespace vulkan;
  using namespace byte_literals;
  vkComputeDescSetLayout = vkLogicalDevice->createDescriptorSetLayout(
      {.bindings = {
           {.binding = 0,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// svo
           {.binding = 1,
            .type = vk::DescriptorType::eUniformBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// debug
           {.binding = 2,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// model infos
           {.binding = 3,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// bvh
           {.binding = 4,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 5,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// probe positions
           {.binding = 5,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// probe textures
           {.binding = 6,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// debug probe texture
       }});

  const auto setLayouts = std::vector{**vkComputeDescSetLayout};
  const auto pipelineLayoutInfo =
      vk::PipelineLayoutCreateInfo{.setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
                                   .pSetLayouts = setLayouts.data()};
  auto computePipelineLayout = (*vkLogicalDevice)->createPipelineLayoutUnique(pipelineLayoutInfo);
  auto allocInfo = vk::DescriptorSetAllocateInfo{};
  allocInfo.setSetLayouts(setLayouts);
  allocInfo.descriptorPool = **vkDescPool;
  computeDescriptorSets = (*vkLogicalDevice)->allocateDescriptorSetsUnique(allocInfo);

  // TODO: size
  probePosBuffer = vkLogicalDevice->createBuffer({.size = 10_MB,
                                                  .usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
                                                  .sharingMode = vk::SharingMode::eExclusive,
                                                  .queueFamilyIndices = {}});
  probePosBuffer->mapping().set(glm::vec4{0, 0, 0, 0});

  debugUniformBuffer = vkLogicalDevice->createBuffer({.size = sizeof(uint32_t) + sizeof(float),
                                                      .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                                                      .sharingMode = vk::SharingMode::eExclusive,
                                                      .queueFamilyIndices = {}});

  const auto svoInfo = vk::DescriptorBufferInfo{.buffer = **svoBuffer, .offset = 0, .range = svoBuffer->getSize()};
  const auto svoWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                               .dstBinding = 0,
                                               .dstArrayElement = {},
                                               .descriptorCount = 1,
                                               .descriptorType = vk::DescriptorType::eStorageBuffer,
                                               .pBufferInfo = &svoInfo};

  const auto uniformDebugInfo =
      vk::DescriptorBufferInfo{.buffer = **debugUniformBuffer, .offset = 0, .range = debugUniformBuffer->getSize()};
  const auto uniformDebugWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                        .dstBinding = 1,
                                                        .dstArrayElement = {},
                                                        .descriptorCount = 1,
                                                        .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                        .pBufferInfo = &uniformDebugInfo};

  const auto modelInfoInfo =
      vk::DescriptorBufferInfo{.buffer = **modelInfoBuffer, .offset = 0, .range = modelInfoBuffer->getSize()};
  const auto modelInfoWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                     .dstBinding = 2,
                                                     .dstArrayElement = {},
                                                     .descriptorCount = 1,
                                                     .descriptorType = vk::DescriptorType::eStorageBuffer,
                                                     .pBufferInfo = &modelInfoInfo};

  const auto bvhInfo = vk::DescriptorBufferInfo{.buffer = **bvhBuffer, .offset = 0, .range = bvhBuffer->getSize()};
  const auto bvhWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                               .dstBinding = 3,
                                               .dstArrayElement = {},
                                               .descriptorCount = 1,
                                               .descriptorType = vk::DescriptorType::eStorageBuffer,
                                               .pBufferInfo = &bvhInfo};

  const auto probePosInfo =
      vk::DescriptorBufferInfo{.buffer = **probePosBuffer, .offset = 0, .range = probePosBuffer->getSize()};
  const auto probePosWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                    .dstBinding = 4,
                                                    .dstArrayElement = {},
                                                    .descriptorCount = 1,
                                                    .descriptorType = vk::DescriptorType::eStorageBuffer,
                                                    .pBufferInfo = &probePosInfo};

  const auto computeProbesInfo = vk::DescriptorImageInfo{.sampler = {},
                                                         .imageView = **vkProbesImageView,
                                                         .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeProbesWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                         .dstBinding = 5,
                                                         .dstArrayElement = {},
                                                         .descriptorCount = 1,
                                                         .descriptorType = vk::DescriptorType::eStorageImage,
                                                         .pImageInfo = &computeProbesInfo};

  const auto computeDebugProbesInfo = vk::DescriptorImageInfo{.sampler = {},
                                                              .imageView = **vkProbesDebugImageView,
                                                              .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeDebugProbesWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                              .dstBinding = 6,
                                                              .dstArrayElement = {},
                                                              .descriptorCount = 1,
                                                              .descriptorType = vk::DescriptorType::eStorageImage,
                                                              .pImageInfo = &computeDebugProbesInfo};

  const auto writeSets = std::vector{svoWrite,      uniformDebugWrite,       modelInfoWrite,    bvhWrite,
                                     probePosWrite, computeDebugProbesWrite, computeProbesWrite};
  (*vkLogicalDevice)->updateDescriptorSets(writeSets, nullptr);

  auto computeShader = vkLogicalDevice->createShader(ShaderConfigGlslFile{
      .name = "light_field_probes",
      .type = ShaderType::Compute,
      .path =
      (std::filesystem::path(*config["resources"]["path_shaders"].value<std::string>()) /= "probes_textures.comp")
          .string(),
      .macros = {}/*,
        .replaceMacros = {{"LOCAL_SIZE_X", std::to_string(computeLocalSize.first)},
                          {"LOCAL_SIZE_Y", std::to_string(computeLocalSize.second)}}*/});

  const auto computeStageInfo = vk::PipelineShaderStageCreateInfo{.stage = computeShader->getVkType(),
                                                                  .module = **computeShader,
                                                                  .pName = "main"};
  const auto pipelineInfo = vk::ComputePipelineCreateInfo{.stage = computeStageInfo, .layout = *computePipelineLayout};
  vkComputePipeline = ComputePipeline::CreateShared(
      (*vkLogicalDevice)->createComputePipelineUnique(nullptr, pipelineInfo).value, std::move(computePipelineLayout));
}
void ProbeRenderer::createCommands() {
  vkCommandPool = vkLogicalDevice->createCommandPool(
      {.queueFamily = vk::QueueFlagBits::eCompute, .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer});

  vkProbesDebugImage->transitionLayout(*vkCommandPool, vk::ImageLayout::eGeneral,
                                       vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
  vkProbesImage->transitionLayout(
      *vkCommandPool, vk::ImageLayout::eGeneral,
      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, PROBES_IMG_ARRAY_LAYERS});

  vkCommandBuffer = vkCommandPool->createCommandBuffers({.level = vk::CommandBufferLevel::ePrimary, .count = 1})[0];
}
void ProbeRenderer::createFences() {
  vkComputeFence = vkLogicalDevice->createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
  vkComputeSemaphore = vkLogicalDevice->createSemaphore();
}
const std::shared_ptr<vulkan::Image> &ProbeRenderer::getProbesImage() const { return vkProbesImage; }
const std::shared_ptr<vulkan::ImageView> &ProbeRenderer::getProbesImageView() const { return vkProbesImageView; }
const std::shared_ptr<vulkan::Image> &ProbeRenderer::getProbesDebugImage() const { return vkProbesDebugImage; }
const std::shared_ptr<vulkan::ImageView> &ProbeRenderer::getProbesDebugImageView() const {
  return vkProbesDebugImageView;
}
const std::shared_ptr<vulkan::TextureSampler> &ProbeRenderer::getProbesDebugSampler() const {
  return vkProbesDebugImageSampler;
}
const std::shared_ptr<vulkan::Buffer> &ProbeRenderer::getProbePosBuffer() const { return probePosBuffer; }
const std::shared_ptr<vulkan::Buffer> &ProbeRenderer::getDebugUniformBuffer() const { return debugUniformBuffer; }
void ProbeRenderer::recordCommands() {
  auto recording = vkCommandBuffer->begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
  recording.bindPipeline(vk::PipelineBindPoint::eCompute, *vkComputePipeline);
  const auto vkDescSets = computeDescriptorSets | ranges::views::transform([](const auto &descSet) { return *descSet; })
      | ranges::to_vector;

  recording.getCommandBuffer()->bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                                   vkComputePipeline->getVkPipelineLayout(), 0, vkDescSets, {});
  recording.dispatch(vkProbesImage->getExtent().width / 8, vkProbesImage->getExtent().height / 8, 1); // FIXME: dynamic group size
  recording.end();
}
const std::shared_ptr<vulkan::Semaphore> &ProbeRenderer::render() {
  vkComputeFence->reset();
  vkCommandBuffer->submit({.waitSemaphores = {},
                           .signalSemaphores = {*vkComputeSemaphore},
                           .flags = {},
                           .fence = *vkComputeFence,
                           .wait = true});
  return vkComputeSemaphore;
}
}// namespace pf::lfp