//
// Created by petr on 6/19/21.
//

#include "ProbeBakeRenderer.h"

#include <utility>

namespace pf::lfp {

ProbeBakeRenderer::ProbeBakeRenderer(
    toml::table config, std::shared_ptr<vulkan::Instance> vkInstance, std::shared_ptr<vulkan::PhysicalDevice> vkDevice,
    std::shared_ptr<vulkan::LogicalDevice> logicalDevice, std::shared_ptr<vulkan::Buffer> svoBuffer,
    std::shared_ptr<vulkan::Buffer> modelInfoBuffer, std::shared_ptr<vulkan::Buffer> bvhBuffer,
    std::shared_ptr<vulkan::Buffer> camBuffer, std::shared_ptr<vulkan::Buffer> materialBuffer,
    std::unique_ptr<ProbeManager> probeManag)
    : config(std::move(config)), vkInstance(std::move(vkInstance)), vkDevice(std::move(vkDevice)),
      vkLogicalDevice(std::move(logicalDevice)), svoBuffer(std::move(svoBuffer)),
      modelInfoBuffer(std::move(modelInfoBuffer)), bvhBuffer(std::move(bvhBuffer)), cameraBuffer(std::move(camBuffer)),
      materialsBuffer(std::move(materialBuffer)), probeManager(std::move(probeManag)) {
  using namespace byte_literals;
  proximityGridData.proximityBuffer =
      vkLogicalDevice->createBuffer({.size = 100_MB,
                                     .usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
                                     .sharingMode = vk::SharingMode::eExclusive,
                                     .queueFamilyIndices = {}});
  proximityGridData.gridInfoBuffer =
      vkLogicalDevice->createBuffer({.size = sizeof(glm::ivec4) * 2,
                                     .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                                     .sharingMode = vk::SharingMode::eExclusive,
                                     .queueFamilyIndices = {}});

  createProbeGenDescriptorPool();
  createTextures();
  createProbeGenPipeline();
  createProbeGenCommands();
  createFences();
  recordProbeGenCommands();

  createSmallProbeGenDescriptorPool();
  createSmallProbeGenPipeline();
  createSmallProbeGenCommands();
  recordSmallProbeGenCommands();

  createProximityDescriptorPool();
  createProximityPipeline();
  createProximityCommands();
  recordProximityCommands();

  createRenderDescriptorPool();
  createRenderPipeline();
  createRenderCommands();
  createFences();
  recordRenderCommands();

  updateGridBuffers();

  setProbeToRender(0);
  setFillHoles(false);
}

void ProbeBakeRenderer::createTextures() {
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
}

void ProbeBakeRenderer::createProbeGenDescriptorPool() {
  probeGenData.vkDescPool =
      vkLogicalDevice->createDescriptorPool({.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                             .maxSets = 1,
                                             .poolSizes = {
                                                 {vk::DescriptorType::eStorageBuffer, 1},// svo
                                                 {vk::DescriptorType::eUniformBuffer, 1},// debug
                                                 {vk::DescriptorType::eStorageBuffer, 1},// model infos
                                                 {vk::DescriptorType::eStorageBuffer, 1},// bvh
                                                 {vk::DescriptorType::eStorageImage, 1}, // probe array
                                                 {vk::DescriptorType::eUniformBuffer, 1},// grid info
                                                 {vk::DescriptorType::eStorageBuffer, 1},// materials
                                             }});
}
void ProbeBakeRenderer::createProbeGenPipeline() {
  using namespace vulkan;
  using namespace byte_literals;
  probeGenData.vkComputeDescSetLayout = vkLogicalDevice->createDescriptorSetLayout(
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
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// probe textures
           {.binding = 5,
            .type = vk::DescriptorType::eUniformBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// grid info
           {.binding = 6,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// materials
       }});

  const auto setLayouts = std::vector{**probeGenData.vkComputeDescSetLayout};
  const auto pipelineLayoutInfo =
      vk::PipelineLayoutCreateInfo{.setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
                                   .pSetLayouts = setLayouts.data()};
  auto computePipelineLayout = (*vkLogicalDevice)->createPipelineLayoutUnique(pipelineLayoutInfo);
  auto allocInfo = vk::DescriptorSetAllocateInfo{};
  allocInfo.setSetLayouts(setLayouts);
  allocInfo.descriptorPool = **probeGenData.vkDescPool;
  probeGenData.computeDescriptorSets = (*vkLogicalDevice)->allocateDescriptorSetsUnique(allocInfo);

  probeGenData.debugUniformBuffer = vkLogicalDevice->createBuffer(
      {.size = sizeof(std::uint32_t) + sizeof(float) + sizeof(std::uint32_t) + sizeof(std::uint32_t),
       .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
       .sharingMode = vk::SharingMode::eExclusive,
       .queueFamilyIndices = {}});

  gridInfoBuffer = vkLogicalDevice->createBuffer({.size = sizeof(glm::vec4) + sizeof(glm::ivec4) + sizeof(glm::vec4),
                                                  .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                                                  .sharingMode = vk::SharingMode::eExclusive,
                                                  .queueFamilyIndices = {}});

  const auto svoInfo = vk::DescriptorBufferInfo{.buffer = **svoBuffer, .offset = 0, .range = svoBuffer->getSize()};
  const auto svoWrite = vk::WriteDescriptorSet{.dstSet = *probeGenData.computeDescriptorSets[0],
                                               .dstBinding = 0,
                                               .dstArrayElement = {},
                                               .descriptorCount = 1,
                                               .descriptorType = vk::DescriptorType::eStorageBuffer,
                                               .pBufferInfo = &svoInfo};

  const auto uniformDebugInfo = vk::DescriptorBufferInfo{.buffer = **probeGenData.debugUniformBuffer,
                                                         .offset = 0,
                                                         .range = probeGenData.debugUniformBuffer->getSize()};
  const auto uniformDebugWrite = vk::WriteDescriptorSet{.dstSet = *probeGenData.computeDescriptorSets[0],
                                                        .dstBinding = 1,
                                                        .dstArrayElement = {},
                                                        .descriptorCount = 1,
                                                        .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                        .pBufferInfo = &uniformDebugInfo};

  const auto modelInfoInfo =
      vk::DescriptorBufferInfo{.buffer = **modelInfoBuffer, .offset = 0, .range = modelInfoBuffer->getSize()};
  const auto modelInfoWrite = vk::WriteDescriptorSet{.dstSet = *probeGenData.computeDescriptorSets[0],
                                                     .dstBinding = 2,
                                                     .dstArrayElement = {},
                                                     .descriptorCount = 1,
                                                     .descriptorType = vk::DescriptorType::eStorageBuffer,
                                                     .pBufferInfo = &modelInfoInfo};

  const auto bvhInfo = vk::DescriptorBufferInfo{.buffer = **bvhBuffer, .offset = 0, .range = bvhBuffer->getSize()};
  const auto bvhWrite = vk::WriteDescriptorSet{.dstSet = *probeGenData.computeDescriptorSets[0],
                                               .dstBinding = 3,
                                               .dstArrayElement = {},
                                               .descriptorCount = 1,
                                               .descriptorType = vk::DescriptorType::eStorageBuffer,
                                               .pBufferInfo = &bvhInfo};

  const auto computeProbesInfo = vk::DescriptorImageInfo{.sampler = {},
                                                         .imageView = **probeManager->getProbesImageView(),
                                                         .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeProbesWrite = vk::WriteDescriptorSet{.dstSet = *probeGenData.computeDescriptorSets[0],
                                                         .dstBinding = 4,
                                                         .dstArrayElement = {},
                                                         .descriptorCount = 1,
                                                         .descriptorType = vk::DescriptorType::eStorageImage,
                                                         .pImageInfo = &computeProbesInfo};

  const auto gridInfoInfo =
      vk::DescriptorBufferInfo{.buffer = **gridInfoBuffer, .offset = 0, .range = gridInfoBuffer->getSize()};

  const auto gridInfoWrite = vk::WriteDescriptorSet{.dstSet = *probeGenData.computeDescriptorSets[0],
                                                    .dstBinding = 5,
                                                    .dstArrayElement = {},
                                                    .descriptorCount = 1,
                                                    .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                    .pBufferInfo = &gridInfoInfo};

  const auto materialsInfo =
      vk::DescriptorBufferInfo{.buffer = **materialsBuffer, .offset = 0, .range = materialsBuffer->getSize()};

  const auto materialsWrite = vk::WriteDescriptorSet{.dstSet = *probeGenData.computeDescriptorSets[0],
                                                     .dstBinding = 6,
                                                     .dstArrayElement = {},
                                                     .descriptorCount = 1,
                                                     .descriptorType = vk::DescriptorType::eStorageBuffer,
                                                     .pBufferInfo = &materialsInfo};

  const auto writeSets = std::vector{svoWrite,           uniformDebugWrite, modelInfoWrite, bvhWrite,
                                     computeProbesWrite, gridInfoWrite,     materialsWrite};
  (*vkLogicalDevice)->updateDescriptorSets(writeSets, nullptr);

  auto computeShader = vkLogicalDevice->createShader(ShaderConfigGlslFile{
      .name = "light_field_probes",
      .type = ShaderType::Compute,
      .path =
      (std::filesystem::path(*config["resources"]["path_shaders"].value<std::string>()) /= "probes_textures_bake_bounces.comp")
          .string(),
      .macros = {}/*,
        .replaceMacros = {{"LOCAL_SIZE_X", std::to_string(computeLocalSize.first)},
                          {"LOCAL_SIZE_Y", std::to_string(computeLocalSize.second)}}*/});

  const auto computeStageInfo = vk::PipelineShaderStageCreateInfo{.stage = computeShader->getVkType(),
                                                                  .module = **computeShader,
                                                                  .pName = "main"};
  const auto pipelineInfo = vk::ComputePipelineCreateInfo{.stage = computeStageInfo, .layout = *computePipelineLayout};
  probeGenData.vkComputePipeline = ComputePipeline::CreateShared(
      (*vkLogicalDevice)->createComputePipelineUnique(nullptr, pipelineInfo).value, std::move(computePipelineLayout));
}
void ProbeBakeRenderer::createProbeGenCommands() {
  probeGenData.vkCommandPool = vkLogicalDevice->createCommandPool(
      {.queueFamily = vk::QueueFlagBits::eCompute, .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer});

  probeManager->getProbesImage()->transitionLayout(
      *probeGenData.vkCommandPool, vk::ImageLayout::eGeneral,
      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, probeManager->getTotalProbeCount()});

  probeGenData.vkCommandBuffer =
      probeGenData.vkCommandPool->createCommandBuffers({.level = vk::CommandBufferLevel::ePrimary, .count = 1})[0];
}
void ProbeBakeRenderer::createFences() {
  vkComputeFence = vkLogicalDevice->createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
  probeGenData.vkComputeSemaphore = vkLogicalDevice->createSemaphore();
  smallProbeGenData.vkComputeSemaphore = vkLogicalDevice->createSemaphore();
  proximityGridData.vkComputeSemaphore = vkLogicalDevice->createSemaphore();
  renderData.vkComputeSemaphore = vkLogicalDevice->createSemaphore();
}
const std::shared_ptr<vulkan::Image> &ProbeBakeRenderer::getProbesDebugImage() const { return vkProbesDebugImage; }
const std::shared_ptr<vulkan::ImageView> &ProbeBakeRenderer::getProbesDebugImageView() const {
  return vkProbesDebugImageView;
}
const std::shared_ptr<vulkan::TextureSampler> &ProbeBakeRenderer::getProbesDebugSampler() const {
  return vkProbesDebugImageSampler;
}

const std::shared_ptr<vulkan::Buffer> &ProbeBakeRenderer::getProbeGenDebugUniformBuffer() const {
  return probeGenData.debugUniformBuffer;
}
void ProbeBakeRenderer::recordProbeGenCommands() {
  auto recording = probeGenData.vkCommandBuffer->begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
  recording.bindPipeline(vk::PipelineBindPoint::eCompute, *probeGenData.vkComputePipeline);
  const auto vkDescSets = probeGenData.computeDescriptorSets
      | ranges::views::transform([](const auto &descSet) { return *descSet; }) | ranges::to_vector;

  recording.getCommandBuffer()->bindDescriptorSets(
      vk::PipelineBindPoint::eCompute, probeGenData.vkComputePipeline->getVkPipelineLayout(), 0, vkDescSets, {});
  recording.dispatch(probeManager->TEXTURE_SIZE.x / 8, probeManager->TEXTURE_SIZE.y / 8,
                     probeManager->getTotalProbeCount());
  recording.end();
}
const std::shared_ptr<vulkan::Semaphore> &ProbeBakeRenderer::renderProbeTextures() {
  vkComputeFence->reset();
  probeGenData.vkCommandBuffer->submit({.waitSemaphores = {},
                                        .signalSemaphores = {*probeGenData.vkComputeSemaphore},
                                        .flags = {},
                                        .fence = *vkComputeFence,
                                        .wait = true});
  vkComputeFence->reset();
  smallProbeGenData.vkCommandBuffer->submit({.waitSemaphores = {*probeGenData.vkComputeSemaphore},
                                             .signalSemaphores = {*smallProbeGenData.vkComputeSemaphore},
                                             .flags = {vk::PipelineStageFlagBits::eComputeShader},
                                             .fence = *vkComputeFence,
                                             .wait = true});
  vkComputeFence->reset();
  proximityGridData.vkCommandBuffer->submit({.waitSemaphores = {*smallProbeGenData.vkComputeSemaphore},
                                             .signalSemaphores = {*proximityGridData.vkComputeSemaphore},
                                             .flags = {vk::PipelineStageFlagBits::eComputeShader},
                                             .fence = *vkComputeFence,
                                             .wait = true});

  return proximityGridData.vkComputeSemaphore;
}
ProbeManager &ProbeBakeRenderer::getProbeManager() { return *probeManager; }
void ProbeBakeRenderer::setProbeToRender(std::uint32_t index) {
  renderData.debugUniformBuffer->mapping().set(index, 1);
}
void ProbeBakeRenderer::setProbeToRender(glm::ivec3 position) {
  const auto probeCount = probeManager->getProbeCount();
  const auto index = (position.x + position.y * probeCount.x + position.z * probeCount.x * probeCount.y);
  setProbeToRender(index);
}
void ProbeBakeRenderer::renderProbesInNextPass() { renderingProbesInNextPass = true; }

void ProbeBakeRenderer::createRenderDescriptorPool() {
  renderData.vkDescPool =
      vkLogicalDevice->createDescriptorPool({.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                             .maxSets = 1,
                                             .poolSizes = {
                                                 {vk::DescriptorType::eUniformBuffer, 1},// debug
                                                 {vk::DescriptorType::eUniformBuffer, 1},// grid info
                                                 {vk::DescriptorType::eStorageImage, 1}, // probe images
                                                 {vk::DescriptorType::eStorageImage, 1}, // probe images small
                                                 {vk::DescriptorType::eStorageImage, 1}, // probe debug image
                                                 {vk::DescriptorType::eUniformBuffer, 1},// camera info
                                                 {vk::DescriptorType::eStorageBuffer, 1},// prox grid data
                                                 {vk::DescriptorType::eUniformBuffer, 1},// prox grid info
                                                 //{vk::DescriptorType::eStorageImage, 1}, // probe images smallest
                                             }});
}
void ProbeBakeRenderer::createRenderPipeline() {
  using namespace vulkan;
  using namespace byte_literals;
  renderData.vkComputeDescSetLayout = vkLogicalDevice->createDescriptorSetLayout(
      {.bindings = {
           {.binding = 0,
            .type = vk::DescriptorType::eUniformBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// debug
           {.binding = 1,
            .type = vk::DescriptorType::eUniformBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// grid info
           {.binding = 2,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// probe images
           {.binding = 3,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// probe images small
           {.binding = 4,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// probe debug image
           {.binding = 5,
            .type = vk::DescriptorType::eUniformBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// camera info
           {.binding = 6,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// prox grid data
           {.binding = 7,
            .type = vk::DescriptorType::eUniformBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// prox grid info
                                                             /*{.binding = 8,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// probe images smallest*/
       }});

  const auto setLayouts = std::vector{**renderData.vkComputeDescSetLayout};
  const auto pipelineLayoutInfo =
      vk::PipelineLayoutCreateInfo{.setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
                                   .pSetLayouts = setLayouts.data()};
  auto computePipelineLayout = (*vkLogicalDevice)->createPipelineLayoutUnique(pipelineLayoutInfo);
  auto allocInfo = vk::DescriptorSetAllocateInfo{};
  allocInfo.setSetLayouts(setLayouts);
  allocInfo.descriptorPool = **renderData.vkDescPool;
  renderData.computeDescriptorSets = (*vkLogicalDevice)->allocateDescriptorSetsUnique(allocInfo);

  renderData.debugUniformBuffer =
      vkLogicalDevice->createBuffer({.size = sizeof(std::uint32_t) + sizeof(std::uint32_t) + sizeof(int) * 2,
                                     .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                                     .sharingMode = vk::SharingMode::eExclusive,
                                     .queueFamilyIndices = {}});

  const auto uniformDebugInfo = vk::DescriptorBufferInfo{.buffer = **renderData.debugUniformBuffer,
                                                         .offset = 0,
                                                         .range = renderData.debugUniformBuffer->getSize()};
  const auto uniformDebugWrite = vk::WriteDescriptorSet{.dstSet = *renderData.computeDescriptorSets[0],
                                                        .dstBinding = 0,
                                                        .dstArrayElement = {},
                                                        .descriptorCount = 1,
                                                        .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                        .pBufferInfo = &uniformDebugInfo};

  const auto gridInfoInfo =
      vk::DescriptorBufferInfo{.buffer = **gridInfoBuffer, .offset = 0, .range = gridInfoBuffer->getSize()};
  const auto gridInfoWrite = vk::WriteDescriptorSet{.dstSet = *renderData.computeDescriptorSets[0],
                                                    .dstBinding = 1,
                                                    .dstArrayElement = {},
                                                    .descriptorCount = 1,
                                                    .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                    .pBufferInfo = &gridInfoInfo};

  const auto computeProbesInfo = vk::DescriptorImageInfo{.sampler = {},
                                                         .imageView = **probeManager->getProbesImageView(),
                                                         .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeProbesWrite = vk::WriteDescriptorSet{.dstSet = *renderData.computeDescriptorSets[0],
                                                         .dstBinding = 2,
                                                         .dstArrayElement = {},
                                                         .descriptorCount = 1,
                                                         .descriptorType = vk::DescriptorType::eStorageImage,
                                                         .pImageInfo = &computeProbesInfo};

  const auto computeSmallProbesInfo = vk::DescriptorImageInfo{.sampler = {},
                                                              .imageView = **probeManager->getProbesImageViewSmall(),
                                                              .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeSmallProbesWrite = vk::WriteDescriptorSet{.dstSet = *renderData.computeDescriptorSets[0],
                                                              .dstBinding = 3,
                                                              .dstArrayElement = {},
                                                              .descriptorCount = 1,
                                                              .descriptorType = vk::DescriptorType::eStorageImage,
                                                              .pImageInfo = &computeSmallProbesInfo};

  const auto computeDebugProbesInfo = vk::DescriptorImageInfo{.sampler = {},
                                                              .imageView = **vkProbesDebugImageView,
                                                              .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeDebugProbesWrite = vk::WriteDescriptorSet{.dstSet = *renderData.computeDescriptorSets[0],
                                                              .dstBinding = 4,
                                                              .dstArrayElement = {},
                                                              .descriptorCount = 1,
                                                              .descriptorType = vk::DescriptorType::eStorageImage,
                                                              .pImageInfo = &computeDebugProbesInfo};

  const auto cameraDebugInfo =
      vk::DescriptorBufferInfo{.buffer = **cameraBuffer, .offset = 0, .range = cameraBuffer->getSize()};
  const auto cameraDebugWrite = vk::WriteDescriptorSet{.dstSet = *renderData.computeDescriptorSets[0],
                                                       .dstBinding = 5,
                                                       .dstArrayElement = {},
                                                       .descriptorCount = 1,
                                                       .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                       .pBufferInfo = &cameraDebugInfo};

  const auto proxGridInfo = vk::DescriptorBufferInfo{.buffer = **proximityGridData.proximityBuffer,
                                                     .offset = 0,
                                                     .range = proximityGridData.proximityBuffer->getSize()};
  const auto proxGridWrite = vk::WriteDescriptorSet{.dstSet = *renderData.computeDescriptorSets[0],
                                                    .dstBinding = 6,
                                                    .dstArrayElement = {},
                                                    .descriptorCount = 1,
                                                    .descriptorType = vk::DescriptorType::eStorageBuffer,
                                                    .pBufferInfo = &proxGridInfo};

  const auto proxGridInfoInfo = vk::DescriptorBufferInfo{.buffer = **proximityGridData.gridInfoBuffer,
                                                         .offset = 0,
                                                         .range = proximityGridData.gridInfoBuffer->getSize()};
  const auto proxGridInfoWrite = vk::WriteDescriptorSet{.dstSet = *renderData.computeDescriptorSets[0],
                                                        .dstBinding = 7,
                                                        .dstArrayElement = {},
                                                        .descriptorCount = 1,
                                                        .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                        .pBufferInfo = &proxGridInfoInfo};

  /* const auto computeSmallestProbesInfo =
      vk::DescriptorImageInfo{.sampler = {},
                              .imageView = **probeManager->getProbesImageViewSmallest(),
                              .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeSmallestProbesWrite = vk::WriteDescriptorSet{.dstSet = *renderData.computeDescriptorSets[0],
                                                                 .dstBinding = 8,
                                                                 .dstArrayElement = {},
                                                                 .descriptorCount = 1,
                                                                 .descriptorType = vk::DescriptorType::eStorageImage,
                                                                 .pImageInfo = &computeSmallestProbesInfo};*/

  const auto writeSets = std::vector{uniformDebugWrite,       computeDebugProbesWrite, computeProbesWrite,
                                     computeSmallProbesWrite, gridInfoWrite,           cameraDebugWrite,
                                     proxGridWrite,           proxGridInfoWrite /*,       computeSmallestProbesWrite*/};
  (*vkLogicalDevice)->updateDescriptorSets(writeSets, nullptr);

  auto computeShader = vkLogicalDevice->createShader(ShaderConfigGlslFile{
      .name = "light_field_probes_render",
      .type = ShaderType::Compute,
      .path =
      (std::filesystem::path(*config["resources"]["path_shaders"].value<std::string>()) /= "probes_render.comp")
          .string(),
      .macros = {}/*,
        .replaceMacros = {{"LOCAL_SIZE_X", std::to_string(computeLocalSize.first)},
                          {"LOCAL_SIZE_Y", std::to_string(computeLocalSize.second)}}*/});

  const auto computeStageInfo = vk::PipelineShaderStageCreateInfo{.stage = computeShader->getVkType(),
                                                                  .module = **computeShader,
                                                                  .pName = "main"};
  const auto pipelineInfo = vk::ComputePipelineCreateInfo{.stage = computeStageInfo, .layout = *computePipelineLayout};
  renderData.vkComputePipeline = ComputePipeline::CreateShared(
      (*vkLogicalDevice)->createComputePipelineUnique(nullptr, pipelineInfo).value, std::move(computePipelineLayout));
}

void ProbeBakeRenderer::createRenderCommands() {
  renderData.vkCommandPool = vkLogicalDevice->createCommandPool(
      {.queueFamily = vk::QueueFlagBits::eCompute, .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer});

  vkProbesDebugImage->transitionLayout(*renderData.vkCommandPool, vk::ImageLayout::eGeneral,
                                       vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

  renderData.vkCommandBuffer =
      renderData.vkCommandPool->createCommandBuffers({.level = vk::CommandBufferLevel::ePrimary, .count = 1})[0];
}
void ProbeBakeRenderer::recordRenderCommands() {
  auto recording = renderData.vkCommandBuffer->begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
  recording.bindPipeline(vk::PipelineBindPoint::eCompute, *renderData.vkComputePipeline);
  const auto vkDescSets = renderData.computeDescriptorSets
      | ranges::views::transform([](const auto &descSet) { return *descSet; }) | ranges::to_vector;

  recording.getCommandBuffer()->bindDescriptorSets(
      vk::PipelineBindPoint::eCompute, renderData.vkComputePipeline->getVkPipelineLayout(), 0, vkDescSets, {});
  recording.dispatch(probeManager->TEXTURE_SIZE.x / 8, probeManager->TEXTURE_SIZE.y / 8, 1);
  recording.end();
}
const std::shared_ptr<vulkan::Semaphore> &ProbeBakeRenderer::render() {
  std::vector<std::reference_wrapper<vulkan::Semaphore>> semaphores;
  //renderingProbesInNextPass= true;
  if (renderingProbesInNextPass) {
    semaphores.emplace_back(*renderProbeTextures());
    renderingProbesInNextPass = false;
  }
  std::vector<vk::PipelineStageFlags> semaphoreFlags{semaphores.size(), vk::PipelineStageFlagBits::eComputeShader};
  vkComputeFence->reset();
  renderData.vkCommandBuffer->submit({.waitSemaphores = semaphores,
                                      .signalSemaphores = {*renderData.vkComputeSemaphore},
                                      .flags = semaphoreFlags,
                                      .fence = *vkComputeFence,
                                      .wait = true});

  return renderData.vkComputeSemaphore;
}
const std::shared_ptr<vulkan::Buffer> &ProbeBakeRenderer::getRenderDebugUniformBuffer() const {
  return renderData.debugUniformBuffer;
}
void ProbeBakeRenderer::setProbeDebugRenderType(ProbeVisualisation type) {
  renderData.debugUniformBuffer->mapping().set(static_cast<uint32_t>(type));
}
void ProbeBakeRenderer::setShaderDebugInt(std::int32_t value) {
  renderData.debugUniformBuffer->mapping().set(value, 2);
}

void ProbeBakeRenderer::createSmallProbeGenDescriptorPool() {
  smallProbeGenData.vkDescPool =
      vkLogicalDevice->createDescriptorPool({.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                             .maxSets = 1,
                                             .poolSizes = {
                                                 {vk::DescriptorType::eStorageImage, 1},// probe array
                                                 {vk::DescriptorType::eStorageImage, 1},// small probe depth array
                                                 {vk::DescriptorType::eStorageImage, 1},// smallest probe depth array
                                             }});
}
void ProbeBakeRenderer::createSmallProbeGenPipeline() {
  using namespace vulkan;
  using namespace byte_literals;
  smallProbeGenData.vkComputeDescSetLayout = vkLogicalDevice->createDescriptorSetLayout(
      {.bindings = {
           {.binding = 0,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// probe textures
           {.binding = 1,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// small probe depth textures
           {.binding = 2,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// smallest probe depth textures
       }});

  const auto setLayouts = std::vector{**smallProbeGenData.vkComputeDescSetLayout};
  const auto pipelineLayoutInfo =
      vk::PipelineLayoutCreateInfo{.setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
                                   .pSetLayouts = setLayouts.data()};
  auto computePipelineLayout = (*vkLogicalDevice)->createPipelineLayoutUnique(pipelineLayoutInfo);
  auto allocInfo = vk::DescriptorSetAllocateInfo{};
  allocInfo.setSetLayouts(setLayouts);
  allocInfo.descriptorPool = **smallProbeGenData.vkDescPool;
  smallProbeGenData.computeDescriptorSets = (*vkLogicalDevice)->allocateDescriptorSetsUnique(allocInfo);

  const auto computeProbesInfo = vk::DescriptorImageInfo{.sampler = {},
                                                         .imageView = **probeManager->getProbesImageView(),
                                                         .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeProbesWrite = vk::WriteDescriptorSet{.dstSet = *smallProbeGenData.computeDescriptorSets[0],
                                                         .dstBinding = 0,
                                                         .dstArrayElement = {},
                                                         .descriptorCount = 1,
                                                         .descriptorType = vk::DescriptorType::eStorageImage,
                                                         .pImageInfo = &computeProbesInfo};

  const auto computeSmallProbesInfo = vk::DescriptorImageInfo{.sampler = {},
                                                              .imageView = **probeManager->getProbesImageViewSmall(),
                                                              .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeSmallProbesWrite = vk::WriteDescriptorSet{.dstSet = *smallProbeGenData.computeDescriptorSets[0],
                                                              .dstBinding = 1,
                                                              .dstArrayElement = {},
                                                              .descriptorCount = 1,
                                                              .descriptorType = vk::DescriptorType::eStorageImage,
                                                              .pImageInfo = &computeSmallProbesInfo};

  const auto computeSmallestProbesInfo =
      vk::DescriptorImageInfo{.sampler = {},
                              .imageView = **probeManager->getProbesImageViewSmallest(),
                              .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeSmallestProbesWrite = vk::WriteDescriptorSet{.dstSet = *smallProbeGenData.computeDescriptorSets[0],
                                                                 .dstBinding = 2,
                                                                 .dstArrayElement = {},
                                                                 .descriptorCount = 1,
                                                                 .descriptorType = vk::DescriptorType::eStorageImage,
                                                                 .pImageInfo = &computeSmallestProbesInfo};

  const auto writeSets = std::vector{computeProbesWrite, computeSmallProbesWrite, computeSmallestProbesWrite};
  (*vkLogicalDevice)->updateDescriptorSets(writeSets, nullptr);

  auto computeShader = vkLogicalDevice->createShader(ShaderConfigGlslFile{
      .name = "light_field_probes_mip",
      .type = ShaderType::Compute,
      .path =
      (std::filesystem::path(*config["resources"]["path_shaders"].value<std::string>()) /= "probes_textures_sample_small.comp")
          .string(),
      .macros = {}/*,
        .replaceMacros = {{"LOCAL_SIZE_X", std::to_string(computeLocalSize.first)},
                          {"LOCAL_SIZE_Y", std::to_string(computeLocalSize.second)}}*/});

  const auto computeStageInfo = vk::PipelineShaderStageCreateInfo{.stage = computeShader->getVkType(),
                                                                  .module = **computeShader,
                                                                  .pName = "main"};
  const auto pipelineInfo = vk::ComputePipelineCreateInfo{.stage = computeStageInfo, .layout = *computePipelineLayout};
  smallProbeGenData.vkComputePipeline = ComputePipeline::CreateShared(
      (*vkLogicalDevice)->createComputePipelineUnique(nullptr, pipelineInfo).value, std::move(computePipelineLayout));
}
void ProbeBakeRenderer::createSmallProbeGenCommands() {
  smallProbeGenData.vkCommandPool = vkLogicalDevice->createCommandPool(
      {.queueFamily = vk::QueueFlagBits::eCompute, .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer});

  probeManager->getProbesImageSmall()->transitionLayout(
      *smallProbeGenData.vkCommandPool, vk::ImageLayout::eGeneral,
      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, probeManager->getTotalProbeCount()});
  probeManager->getProbesImageSmallest()->transitionLayout(
      *smallProbeGenData.vkCommandPool, vk::ImageLayout::eGeneral,
      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, probeManager->getTotalProbeCount()});

  smallProbeGenData.vkCommandBuffer =
      smallProbeGenData.vkCommandPool->createCommandBuffers({.level = vk::CommandBufferLevel::ePrimary, .count = 1})[0];
}
void ProbeBakeRenderer::recordSmallProbeGenCommands() {
  auto recording = smallProbeGenData.vkCommandBuffer->begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
  recording.bindPipeline(vk::PipelineBindPoint::eCompute, *smallProbeGenData.vkComputePipeline);
  const auto vkDescSets = smallProbeGenData.computeDescriptorSets
      | ranges::views::transform([](const auto &descSet) { return *descSet; }) | ranges::to_vector;

  recording.getCommandBuffer()->bindDescriptorSets(
      vk::PipelineBindPoint::eCompute, smallProbeGenData.vkComputePipeline->getVkPipelineLayout(), 0, vkDescSets, {});
  recording.dispatch(probeManager->TEXTURE_SIZE_SMALL.x / 8, probeManager->TEXTURE_SIZE_SMALL.y / 8,
                     probeManager->getTotalProbeCount());
  recording.end();
}
void ProbeBakeRenderer::createProximityDescriptorPool() {
  proximityGridData.vkDescPool =
      vkLogicalDevice->createDescriptorPool({.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                             .maxSets = 1,
                                             .poolSizes = {
                                                 {vk::DescriptorType::eUniformBuffer, 1},// probe grid info
                                                 {vk::DescriptorType::eUniformBuffer, 1},// prox. grid info
                                                 {vk::DescriptorType::eStorageImage, 1}, // probe array
                                                 {vk::DescriptorType::eStorageImage, 1}, // small probe depth array
                                                 {vk::DescriptorType::eStorageBuffer, 1},// output data
                                             }});
}
void ProbeBakeRenderer::createProximityPipeline() {
  using namespace vulkan;
  using namespace byte_literals;
  proximityGridData.vkComputeDescSetLayout = vkLogicalDevice->createDescriptorSetLayout(
      {.bindings = {
           {.binding = 0,
            .type = vk::DescriptorType::eUniformBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// probe grid info
           {.binding = 1,
            .type = vk::DescriptorType::eUniformBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// prox. grid info
           {.binding = 2,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// probe textures
           {.binding = 3,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// small probe depth textures
           {.binding = 4,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// output
       }});

  const auto setLayouts = std::vector{**proximityGridData.vkComputeDescSetLayout};
  const auto pipelineLayoutInfo =
      vk::PipelineLayoutCreateInfo{.setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
                                   .pSetLayouts = setLayouts.data()};
  auto computePipelineLayout = (*vkLogicalDevice)->createPipelineLayoutUnique(pipelineLayoutInfo);
  auto allocInfo = vk::DescriptorSetAllocateInfo{};
  allocInfo.setSetLayouts(setLayouts);
  allocInfo.descriptorPool = **proximityGridData.vkDescPool;
  proximityGridData.computeDescriptorSets = (*vkLogicalDevice)->allocateDescriptorSetsUnique(allocInfo);

  const auto probeGridInfoInfo =
      vk::DescriptorBufferInfo{.buffer = **gridInfoBuffer, .offset = 0, .range = gridInfoBuffer->getSize()};
  const auto probeGridInfoWrite = vk::WriteDescriptorSet{.dstSet = *proximityGridData.computeDescriptorSets[0],
                                                         .dstBinding = 0,
                                                         .dstArrayElement = {},
                                                         .descriptorCount = 1,
                                                         .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                         .pBufferInfo = &probeGridInfoInfo};

  const auto proxGridInfoInfo = vk::DescriptorBufferInfo{.buffer = **proximityGridData.gridInfoBuffer,
                                                         .offset = 0,
                                                         .range = proximityGridData.gridInfoBuffer->getSize()};
  const auto proxGridInfoWrite = vk::WriteDescriptorSet{.dstSet = *proximityGridData.computeDescriptorSets[0],
                                                        .dstBinding = 1,
                                                        .dstArrayElement = {},
                                                        .descriptorCount = 1,
                                                        .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                        .pBufferInfo = &proxGridInfoInfo};

  const auto computeProbesInfo = vk::DescriptorImageInfo{.sampler = {},
                                                         .imageView = **probeManager->getProbesImageView(),
                                                         .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeProbesWrite = vk::WriteDescriptorSet{.dstSet = *proximityGridData.computeDescriptorSets[0],
                                                         .dstBinding = 2,
                                                         .dstArrayElement = {},
                                                         .descriptorCount = 1,
                                                         .descriptorType = vk::DescriptorType::eStorageImage,
                                                         .pImageInfo = &computeProbesInfo};

  const auto computeSmallProbesInfo = vk::DescriptorImageInfo{.sampler = {},
                                                              .imageView = **probeManager->getProbesImageViewSmall(),
                                                              .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeSmallProbesWrite = vk::WriteDescriptorSet{.dstSet = *proximityGridData.computeDescriptorSets[0],
                                                              .dstBinding = 3,
                                                              .dstArrayElement = {},
                                                              .descriptorCount = 1,
                                                              .descriptorType = vk::DescriptorType::eStorageImage,
                                                              .pImageInfo = &computeSmallProbesInfo};

  const auto outputInfo = vk::DescriptorBufferInfo{.buffer = **proximityGridData.proximityBuffer,
                                                   .offset = 0,
                                                   .range = proximityGridData.proximityBuffer->getSize()};
  const auto outputWrite = vk::WriteDescriptorSet{.dstSet = *proximityGridData.computeDescriptorSets[0],
                                                  .dstBinding = 4,
                                                  .dstArrayElement = {},
                                                  .descriptorCount = 1,
                                                  .descriptorType = vk::DescriptorType::eStorageBuffer,
                                                  .pBufferInfo = &outputInfo};

  const auto writeSets =
      std::vector{probeGridInfoWrite, proxGridInfoWrite, computeProbesWrite, computeSmallProbesWrite, outputWrite};
  (*vkLogicalDevice)->updateDescriptorSets(writeSets, nullptr);

  auto computeShader = vkLogicalDevice->createShader(ShaderConfigGlslFile{
      .name = "light_field_probes_prox_grid",
      .type = ShaderType::Compute,
      .path =
      (std::filesystem::path(*config["resources"]["path_shaders"].value<std::string>()) /= "probe_approx_grid.comp")
          .string(),
      .macros = {}/*,
        .replaceMacros = {{"LOCAL_SIZE_X", std::to_string(computeLocalSize.first)},
                          {"LOCAL_SIZE_Y", std::to_string(computeLocalSize.second)}}*/});

  const auto computeStageInfo = vk::PipelineShaderStageCreateInfo{.stage = computeShader->getVkType(),
                                                                  .module = **computeShader,
                                                                  .pName = "main"};
  const auto pipelineInfo = vk::ComputePipelineCreateInfo{.stage = computeStageInfo, .layout = *computePipelineLayout};
  proximityGridData.vkComputePipeline = ComputePipeline::CreateShared(
      (*vkLogicalDevice)->createComputePipelineUnique(nullptr, pipelineInfo).value, std::move(computePipelineLayout));
}
void ProbeBakeRenderer::createProximityCommands() {
  proximityGridData.vkCommandPool = vkLogicalDevice->createCommandPool(
      {.queueFamily = vk::QueueFlagBits::eCompute, .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer});

  proximityGridData.vkCommandBuffer =
      proximityGridData.vkCommandPool->createCommandBuffers({.level = vk::CommandBufferLevel::ePrimary, .count = 1})[0];
}
void ProbeBakeRenderer::recordProximityCommands() {
  auto recording = proximityGridData.vkCommandBuffer->begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
  recording.bindPipeline(vk::PipelineBindPoint::eCompute, *proximityGridData.vkComputePipeline);
  const auto vkDescSets = proximityGridData.computeDescriptorSets
      | ranges::views::transform([](const auto &descSet) { return *descSet; }) | ranges::to_vector;

  recording.getCommandBuffer()->bindDescriptorSets(
      vk::PipelineBindPoint::eCompute, proximityGridData.vkComputePipeline->getVkPipelineLayout(), 0, vkDescSets, {});
  const auto proxGridSize = probeManager->getProximityGridSize();
  recording.dispatch(proxGridSize.x / 8, proxGridSize.y / 8, proxGridSize.z / 8);
  recording.end();
}
void ProbeBakeRenderer::setFillHoles(bool fillHoles) {
  renderData.debugUniformBuffer->mapping().set(fillHoles ? 1 : 0, 3);
}
const std::shared_ptr<vulkan::Buffer> &ProbeBakeRenderer::getProximityBuffer() const {
  return proximityGridData.proximityBuffer;
}
const std::shared_ptr<vulkan::Buffer> &ProbeBakeRenderer::getProximityInfoBuffer() const {
  return proximityGridData.gridInfoBuffer;
}
const std::shared_ptr<vulkan::Buffer> &ProbeBakeRenderer::getGridInfoBuffer() const { return gridInfoBuffer; }

void ProbeBakeRenderer::setGridStart(const glm::vec3 &gridStart) {
  probeManager->setGridStart(gridStart);
  updateGridBuffers();
}
void ProbeBakeRenderer::setGridStep(float gridStep) {
  probeManager->setGridStep(gridStep);
  updateGridBuffers();
}
void ProbeBakeRenderer::setProximityGridSize(const glm::ivec3 &proximityGridSize) {
  probeManager->setProximityGridSize(proximityGridSize);
  updateGridBuffers();
}
void ProbeBakeRenderer::updateGridBuffers() {
  auto gridInfoMapping = gridInfoBuffer->mapping();
  gridInfoMapping.set(glm::ivec4{probeManager->getProbeCount(), 0});
  gridInfoMapping.set(glm::vec4{probeManager->getGridStart(), 0}, 1);
  gridInfoMapping.set(glm::vec4{probeManager->getGridStep(), 0, 0, 0}, 2);

  auto proxGridInfoMapping = proximityGridData.gridInfoBuffer->mapping();
  proxGridInfoMapping.set(glm::ivec4{probeManager->getProximityGridSize(), 0});
  proxGridInfoMapping.set(glm::vec4{probeManager->getProximityGridStep(), 0}, 1);
}
}// namespace pf::lfp