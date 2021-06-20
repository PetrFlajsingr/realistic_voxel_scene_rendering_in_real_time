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
                             std::shared_ptr<vulkan::Buffer> bvhBuffer, std::unique_ptr<ProbeManager> probeManag)
    : config(std::move(config)), vkInstance(std::move(vkInstance)), vkDevice(std::move(vkDevice)),
      vkLogicalDevice(std::move(vkLogicalDevice)), svoBuffer(std::move(svoBuffer)),
      modelInfoBuffer(std::move(modelInfoBuffer)), bvhBuffer(std::move(bvhBuffer)),
      probeManager(std::move(probeManag)) {
  createProbeGenDescriptorPool();
  createTextures();
  createProbeGenPipeline();
  createProbeGenCommands();
  createFences();
  recordProbeGenCommands();

  createRenderDescriptorPool();
  createRenderPipeline();
  createRenderCommands();
  createFences();
  recordRenderCommands();

  auto gridInfoMapping = gridInfoBuffer->mapping();
  gridInfoMapping.set(glm::ivec4{probeManager->getProbeCount(), 0});
  gridInfoMapping.set(glm::vec4{probeManager->getGridStart(), 0}, 1);
  gridInfoMapping.set(glm::vec4{probeManager->getGridStep(), 0, 0, 0}, 2);

  setProbeToRender(0);
}

void ProbeRenderer::createProbeGenDescriptorPool() {
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
}
void ProbeRenderer::createProbeGenPipeline() {
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

  const auto writeSets =
      std::vector{svoWrite, uniformDebugWrite, modelInfoWrite, bvhWrite, computeProbesWrite, gridInfoWrite};
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
  probeGenData.vkComputePipeline = ComputePipeline::CreateShared(
      (*vkLogicalDevice)->createComputePipelineUnique(nullptr, pipelineInfo).value, std::move(computePipelineLayout));
}
void ProbeRenderer::createProbeGenCommands() {
  probeGenData.vkCommandPool = vkLogicalDevice->createCommandPool(
      {.queueFamily = vk::QueueFlagBits::eCompute, .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer});

  probeManager->getProbesImage()->transitionLayout(
      *probeGenData.vkCommandPool, vk::ImageLayout::eGeneral,
      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, probeManager->getTotalProbeCount()});

  probeGenData.vkCommandBuffer =
      probeGenData.vkCommandPool->createCommandBuffers({.level = vk::CommandBufferLevel::ePrimary, .count = 1})[0];
}
void ProbeRenderer::createFences() {
  vkComputeFence = vkLogicalDevice->createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
  probeGenData.vkComputeSemaphore = vkLogicalDevice->createSemaphore();
  renderData.vkComputeSemaphore = vkLogicalDevice->createSemaphore();
}
const std::shared_ptr<vulkan::Image> &ProbeRenderer::getProbesDebugImage() const { return vkProbesDebugImage; }
const std::shared_ptr<vulkan::ImageView> &ProbeRenderer::getProbesDebugImageView() const {
  return vkProbesDebugImageView;
}
const std::shared_ptr<vulkan::TextureSampler> &ProbeRenderer::getProbesDebugSampler() const {
  return vkProbesDebugImageSampler;
}

const std::shared_ptr<vulkan::Buffer> &ProbeRenderer::getProbeGenDebugUniformBuffer() const {
  return probeGenData.debugUniformBuffer;
}
void ProbeRenderer::recordProbeGenCommands() {
  auto recording = probeGenData.vkCommandBuffer->begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
  recording.bindPipeline(vk::PipelineBindPoint::eCompute, *probeGenData.vkComputePipeline);
  const auto vkDescSets = probeGenData.computeDescriptorSets
      | ranges::views::transform([](const auto &descSet) { return *descSet; }) | ranges::to_vector;

  recording.getCommandBuffer()->bindDescriptorSets(
      vk::PipelineBindPoint::eCompute, probeGenData.vkComputePipeline->getVkPipelineLayout(), 0, vkDescSets, {});
  recording.dispatch(probeManager->TEXTURE_SIZE.x / 8, probeManager->TEXTURE_SIZE.y / 8,
                     probeManager->getTotalProbeCount());// FIXME: dynamic group size
  recording.end();
}
const std::shared_ptr<vulkan::Semaphore> &ProbeRenderer::renderProbeTextures() {
  vkComputeFence->reset();
  probeGenData.vkCommandBuffer->submit({.waitSemaphores = {},
                                        .signalSemaphores = {*probeGenData.vkComputeSemaphore},
                                        .flags = {},
                                        .fence = *vkComputeFence,
                                        .wait = true});

  return probeGenData.vkComputeSemaphore;
}
ProbeManager &ProbeRenderer::getProbeManager() { return *probeManager; }
void ProbeRenderer::setProbeToRender(std::uint32_t index) { renderData.debugUniformBuffer->mapping().set(index, 1); }
void ProbeRenderer::setProbeToRender(glm::ivec3 position) {
  const auto probeCount = probeManager->getProbeCount();
  const auto index = (position.x + position.y * probeCount.x + position.z * probeCount.x * probeCount.y);
  setProbeToRender(index);
}
void ProbeRenderer::renderProbesInNextPass() { renderingProbesInNextPass = true; }

void ProbeRenderer::createRenderDescriptorPool() {
  renderData.vkDescPool =
      vkLogicalDevice->createDescriptorPool({.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                             .maxSets = 1,
                                             .poolSizes = {
                                                 {vk::DescriptorType::eUniformBuffer, 1},// debug
                                                 {vk::DescriptorType::eUniformBuffer, 1},// grid info
                                                 {vk::DescriptorType::eStorageImage, 1}, // probe images
                                                 {vk::DescriptorType::eStorageImage, 1}, // probe debug image
                                             }});
}
void ProbeRenderer::createRenderPipeline() {
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
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// probe debug image
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

  renderData.debugUniformBuffer = vkLogicalDevice->createBuffer({.size = sizeof(std::uint32_t) + sizeof(std::uint32_t),
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

  const auto computeDebugProbesInfo = vk::DescriptorImageInfo{.sampler = {},
                                                              .imageView = **vkProbesDebugImageView,
                                                              .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeDebugProbesWrite = vk::WriteDescriptorSet{.dstSet = *renderData.computeDescriptorSets[0],
                                                              .dstBinding = 3,
                                                              .dstArrayElement = {},
                                                              .descriptorCount = 1,
                                                              .descriptorType = vk::DescriptorType::eStorageImage,
                                                              .pImageInfo = &computeDebugProbesInfo};

  const auto writeSets = std::vector{uniformDebugWrite, computeDebugProbesWrite, computeProbesWrite, gridInfoWrite};
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

void ProbeRenderer::createRenderCommands() {
  renderData.vkCommandPool = vkLogicalDevice->createCommandPool(
      {.queueFamily = vk::QueueFlagBits::eCompute, .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer});

  vkProbesDebugImage->transitionLayout(*renderData.vkCommandPool, vk::ImageLayout::eGeneral,
                                       vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

  renderData.vkCommandBuffer =
      renderData.vkCommandPool->createCommandBuffers({.level = vk::CommandBufferLevel::ePrimary, .count = 1})[0];
}
void ProbeRenderer::recordRenderCommands() {
  auto recording = renderData.vkCommandBuffer->begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
  recording.bindPipeline(vk::PipelineBindPoint::eCompute, *renderData.vkComputePipeline);
  const auto vkDescSets = renderData.computeDescriptorSets
      | ranges::views::transform([](const auto &descSet) { return *descSet; }) | ranges::to_vector;

  recording.getCommandBuffer()->bindDescriptorSets(
      vk::PipelineBindPoint::eCompute, renderData.vkComputePipeline->getVkPipelineLayout(), 0, vkDescSets, {});
  recording.dispatch(probeManager->TEXTURE_SIZE.x / 8, probeManager->TEXTURE_SIZE.y / 8, 1);
  recording.end();
}
const std::shared_ptr<vulkan::Semaphore> &ProbeRenderer::render() {
  std::vector<std::reference_wrapper<vulkan::Semaphore>> semaphores;
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
const std::shared_ptr<vulkan::Buffer> &ProbeRenderer::getRenderDebugUniformBuffer() const {
  return renderData.debugUniformBuffer;
}
void ProbeRenderer::setProbeDebugRenderType(ProbeVisualisation type) {
  renderData.debugUniformBuffer->mapping().set(static_cast<uint32_t>(type));
}
}// namespace pf::lfp