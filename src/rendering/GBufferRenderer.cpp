//
// Created by petr on 6/30/21.
//

#include "GBufferRenderer.h"
#include <pf_glfw_vulkan/vulkan/types/Buffer.h>
#include <pf_glfw_vulkan/vulkan/types/CommandBuffer.h>
#include <pf_glfw_vulkan/vulkan/types/CommandPool.h>
#include <pf_glfw_vulkan/vulkan/types/ComputePipeline.h>
#include <pf_glfw_vulkan/vulkan/types/DescriptorPool.h>
#include <pf_glfw_vulkan/vulkan/types/DescriptorSetLayout.h>
#include <pf_glfw_vulkan/vulkan/types/Fence.h>
#include <pf_glfw_vulkan/vulkan/types/Image.h>
#include <pf_glfw_vulkan/vulkan/types/ImageView.h>
#include <pf_glfw_vulkan/vulkan/types/LogicalDevice.h>
#include <pf_glfw_vulkan/vulkan/types/Semaphore.h>
#include <pf_glfw_vulkan/vulkan/types/Shader.h>
#include <pf_glfw_vulkan/vulkan/types/TextureSampler.h>
#include <range/v3/view/transform.hpp>

namespace pf {

GBufferRenderer::GBufferRenderer(std::filesystem::path shaderDir, vk::Extent2D viewportSize,
                                 std::shared_ptr<vulkan::LogicalDevice> vkLogicalDevice,
                                 const std::shared_ptr<vulkan::CommandPool> &vkCommandPool,
                                 std::shared_ptr<vulkan::Buffer> bufferSVO,
                                 std::shared_ptr<vulkan::Buffer> bufferModelInfo,
                                 std::shared_ptr<vulkan::Buffer> bufferBVH, std::shared_ptr<vulkan::Buffer> bufferLight,
                                 std::shared_ptr<vulkan::Buffer> bufferCamera,
                                 std::shared_ptr<vulkan::Buffer> bufferMaterials, vk::Format presentFormat)
    : logicalDevice(std::move(vkLogicalDevice)), extent2D(viewportSize), shaderPath(std::move(shaderDir)),
      svoBuffer(std::move(bufferSVO)), modelInfoBuffer(std::move(bufferModelInfo)), bvhBuffer(std::move(bufferBVH)),
      lightUniformBuffer(std::move(bufferLight)), cameraUniformBuffer(std::move(bufferCamera)),
      materialsBuffer(std::move(bufferMaterials)) {

  createTextures(presentFormat);
  debugUniformBuffer = logicalDevice->createBuffer({.size = sizeof(uint32_t),
                                                    .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                                                    .sharingMode = vk::SharingMode::eExclusive,
                                                    .queueFamilyIndices = {}});
  createDescriptorPools();
  createPipeline();
  createCommands(*vkCommandPool);
  recordCommands();
  createFences();
  createSemaphores();
}

void GBufferRenderer::createTextures(vk::Format presentFormat) {
  posAndMaterialImage = logicalDevice->createImage(
      {.imageType = vk::ImageType::e2D,
       .format = vk::Format::eR32G32B32A32Sfloat,
       .extent = vk::Extent3D{.width = extent2D.width, .height = extent2D.height, .depth = 1},
       .mipLevels = 1,
       .arrayLayers = 1,
       .sampleCount = vk::SampleCountFlagBits::e1,
       .tiling = vk::ImageTiling::eOptimal,
       .usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc
           | vk::ImageUsageFlagBits::eTransferDst,
       .sharingQueues = {},
       .layout = vk::ImageLayout::eUndefined});
  posAndMaterialImageView =
      posAndMaterialImage->createImageView(vk::ColorSpaceKHR::eSrgbNonlinear, vk::ImageViewType::e2D,
                                           vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

  normalImage = logicalDevice->createImage(
      {.imageType = vk::ImageType::e2D,
       .format = vk::Format::eR32G32Sfloat,
       .extent = vk::Extent3D{.width = extent2D.width, .height = extent2D.height, .depth = 1},
       .mipLevels = 1,
       .arrayLayers = 1,
       .sampleCount = vk::SampleCountFlagBits::e1,
       .tiling = vk::ImageTiling::eOptimal,
       .usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc
           | vk::ImageUsageFlagBits::eTransferDst,
       .sharingQueues = {},
       .layout = vk::ImageLayout::eUndefined});
  normalImageView =
      normalImage->createImageView(vk::ColorSpaceKHR::eSrgbNonlinear, vk::ImageViewType::e2D,
                                   vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

  debugImage = logicalDevice->createImage(
      {.imageType = vk::ImageType::e2D,
       .format = presentFormat,
       .extent = vk::Extent3D{.width = extent2D.width, .height = extent2D.height, .depth = 1},
       .mipLevels = 1,
       .arrayLayers = 1,
       .sampleCount = vk::SampleCountFlagBits::e1,
       .tiling = vk::ImageTiling::eOptimal,
       .usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc
           | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
       .sharingQueues = {},
       .layout = vk::ImageLayout::eUndefined});
  debugImageView = debugImage->createImageView(vk::ColorSpaceKHR::eSrgbNonlinear, vk::ImageViewType::e2D,
                                               vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

  debugImageSampler = vulkan::TextureSampler::CreateShared(
      logicalDevice,
      vulkan::TextureSamplerConfig{
          .magFilter = vk::Filter::eLinear,
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
void GBufferRenderer::createDescriptorPools() {
  descriptorPool =
      logicalDevice->createDescriptorPool({.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                           .maxSets = 1,
                                           .poolSizes = {
                                               {vk::DescriptorType::eStorageImage, 1}, // pos and material out
                                               {vk::DescriptorType::eStorageImage, 1}, // normals out
                                               {vk::DescriptorType::eUniformBuffer, 1},// camera
                                               {vk::DescriptorType::eUniformBuffer, 1},// light pos
                                               {vk::DescriptorType::eStorageBuffer, 1},// svo
                                               {vk::DescriptorType::eStorageBuffer, 1},// model infos
                                               {vk::DescriptorType::eStorageBuffer, 1},// bvh
                                               {vk::DescriptorType::eStorageImage, 1}, // debug image
                                               {vk::DescriptorType::eUniformBuffer, 1},// debug data
                                               {vk::DescriptorType::eStorageBuffer, 1},// materials
                                           }});
}
void GBufferRenderer::createPipeline() {
  using namespace vulkan;
  descriptorSetLayout = logicalDevice->createDescriptorSetLayout(
      {.bindings = {
           {.binding = 0,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// pos and material out
           {.binding = 1,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// normals out
           {.binding = 2,
            .type = vk::DescriptorType::eUniformBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},//camera
           {.binding = 3,
            .type = vk::DescriptorType::eUniformBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// light pos
           {.binding = 4,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// svo
           {.binding = 5,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// model infos
           {.binding = 6,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// bvh
           {.binding = 7,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// debug image
           {.binding = 8,
            .type = vk::DescriptorType::eUniformBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},//debug data
           {.binding = 9,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},//debug data
       }});

  const auto setLayouts = std::vector{**descriptorSetLayout};
  const auto pipelineLayoutInfo =
      vk::PipelineLayoutCreateInfo{.setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
                                   .pSetLayouts = setLayouts.data()};
  auto computePipelineLayout = (*logicalDevice)->createPipelineLayoutUnique(pipelineLayoutInfo);
  auto allocInfo = vk::DescriptorSetAllocateInfo{};
  allocInfo.setSetLayouts(setLayouts);
  allocInfo.descriptorPool = **descriptorPool;
  descriptorSets = (*logicalDevice)->allocateDescriptorSetsUnique(allocInfo);

  const auto posAndMaterialInfo = vk::DescriptorImageInfo{.sampler = {},
                                                          .imageView = **posAndMaterialImageView,
                                                          .imageLayout = vk::ImageLayout::eGeneral};
  const auto posAndMaterialWrite = vk::WriteDescriptorSet{.dstSet = *descriptorSets[0],
                                                          .dstBinding = 0,
                                                          .dstArrayElement = {},
                                                          .descriptorCount = 1,
                                                          .descriptorType = vk::DescriptorType::eStorageImage,
                                                          .pImageInfo = &posAndMaterialInfo};

  const auto normalInfo =
      vk::DescriptorImageInfo{.sampler = {}, .imageView = **normalImageView, .imageLayout = vk::ImageLayout::eGeneral};
  const auto normalWrite = vk::WriteDescriptorSet{.dstSet = *descriptorSets[0],
                                                  .dstBinding = 1,
                                                  .dstArrayElement = {},
                                                  .descriptorCount = 1,
                                                  .descriptorType = vk::DescriptorType::eStorageImage,
                                                  .pImageInfo = &normalInfo};

  const auto uniformCameraInfo =
      vk::DescriptorBufferInfo{.buffer = **cameraUniformBuffer, .offset = 0, .range = cameraUniformBuffer->getSize()};
  const auto uniformCameraWrite = vk::WriteDescriptorSet{.dstSet = *descriptorSets[0],
                                                         .dstBinding = 2,
                                                         .dstArrayElement = {},
                                                         .descriptorCount = 1,
                                                         .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                         .pBufferInfo = &uniformCameraInfo};

  const auto lightPosInfo =
      vk::DescriptorBufferInfo{.buffer = **lightUniformBuffer, .offset = 0, .range = lightUniformBuffer->getSize()};
  const auto lightPosWrite = vk::WriteDescriptorSet{.dstSet = *descriptorSets[0],
                                                    .dstBinding = 3,
                                                    .dstArrayElement = {},
                                                    .descriptorCount = 1,
                                                    .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                    .pBufferInfo = &lightPosInfo};

  const auto svoInfo = vk::DescriptorBufferInfo{.buffer = **svoBuffer, .offset = 0, .range = svoBuffer->getSize()};
  const auto svoWrite = vk::WriteDescriptorSet{.dstSet = *descriptorSets[0],
                                               .dstBinding = 4,
                                               .dstArrayElement = {},
                                               .descriptorCount = 1,
                                               .descriptorType = vk::DescriptorType::eStorageBuffer,
                                               .pBufferInfo = &svoInfo};

  const auto modelInfoInfo =
      vk::DescriptorBufferInfo{.buffer = **modelInfoBuffer, .offset = 0, .range = modelInfoBuffer->getSize()};
  const auto modelInfoWrite = vk::WriteDescriptorSet{.dstSet = *descriptorSets[0],
                                                     .dstBinding = 5,
                                                     .dstArrayElement = {},
                                                     .descriptorCount = 1,
                                                     .descriptorType = vk::DescriptorType::eStorageBuffer,
                                                     .pBufferInfo = &modelInfoInfo};

  const auto bvhInfo = vk::DescriptorBufferInfo{.buffer = **bvhBuffer, .offset = 0, .range = bvhBuffer->getSize()};
  const auto bvhWrite = vk::WriteDescriptorSet{.dstSet = *descriptorSets[0],
                                               .dstBinding = 6,
                                               .dstArrayElement = {},
                                               .descriptorCount = 1,
                                               .descriptorType = vk::DescriptorType::eStorageBuffer,
                                               .pBufferInfo = &bvhInfo};

  const auto debugImageInfo =
      vk::DescriptorImageInfo{.sampler = {}, .imageView = **debugImageView, .imageLayout = vk::ImageLayout::eGeneral};
  const auto debugImageWrite = vk::WriteDescriptorSet{.dstSet = *descriptorSets[0],
                                                      .dstBinding = 7,
                                                      .dstArrayElement = {},
                                                      .descriptorCount = 1,
                                                      .descriptorType = vk::DescriptorType::eStorageImage,
                                                      .pImageInfo = &debugImageInfo};

  const auto debugInfo =
      vk::DescriptorBufferInfo{.buffer = **debugUniformBuffer, .offset = 0, .range = debugUniformBuffer->getSize()};
  const auto debugWrite = vk::WriteDescriptorSet{.dstSet = *descriptorSets[0],
                                                 .dstBinding = 8,
                                                 .dstArrayElement = {},
                                                 .descriptorCount = 1,
                                                 .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                 .pBufferInfo = &debugInfo};

  const auto materialsInfo =
      vk::DescriptorBufferInfo{.buffer = **materialsBuffer, .offset = 0, .range = materialsBuffer->getSize()};
  const auto materialsWrite = vk::WriteDescriptorSet{.dstSet = *descriptorSets[0],
                                                     .dstBinding = 9,
                                                     .dstArrayElement = {},
                                                     .descriptorCount = 1,
                                                     .descriptorType = vk::DescriptorType::eStorageBuffer,
                                                     .pBufferInfo = &materialsInfo};

  const auto writeSets =
      std::vector{posAndMaterialWrite, normalWrite, uniformCameraWrite, lightPosWrite, svoWrite,
                  modelInfoWrite,      bvhWrite,    debugImageWrite,    debugWrite,    materialsWrite};
  (*logicalDevice)->updateDescriptorSets(writeSets, nullptr);

  auto computeShader =
      logicalDevice->createShader(ShaderConfigGlslFile{.name = "gbuffer_render",
                                                       .type = ShaderType::Compute,
                                                       .path = (shaderPath / "gbuffer_render.comp").string(),
                                                       .macros = {},
                                                       .replaceMacros = {}});

  const auto computeStageInfo = vk::PipelineShaderStageCreateInfo{.stage = computeShader->getVkType(),
                                                                  .module = **computeShader,
                                                                  .pName = "main"};
  const auto pipelineInfo = vk::ComputePipelineCreateInfo{.stage = computeStageInfo, .layout = *computePipelineLayout};
  computePipeline = ComputePipeline::CreateShared(
      (*logicalDevice)->createComputePipelineUnique(nullptr, pipelineInfo).value, std::move(computePipelineLayout));
}
void GBufferRenderer::createCommands(vulkan::CommandPool &pool) {

  posAndMaterialImage->transitionLayout(pool, vk::ImageLayout::eGeneral,
                                        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
  normalImage->transitionLayout(pool, vk::ImageLayout::eGeneral,
                                vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

  debugImage->transitionLayout(pool, vk::ImageLayout::eGeneral,
                               vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

  commandBuffer = pool.createCommandBuffers({.level = vk::CommandBufferLevel::ePrimary, .count = 1})[0];
}
void GBufferRenderer::recordCommands() {
  auto recording = commandBuffer->begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
  recording.bindPipeline(vk::PipelineBindPoint::eCompute, *computePipeline);
  const auto vkDescSets =
      descriptorSets | ranges::views::transform([](const auto &descSet) { return *descSet; }) | ranges::to_vector;

  recording.getCommandBuffer()->bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                                   computePipeline->getVkPipelineLayout(), 0, vkDescSets, {});
  recording.dispatch(extent2D.width / 8, extent2D.height / 8, 1);
  recording.end();
}

void GBufferRenderer::createFences() {
  fence = logicalDevice->createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
}

void GBufferRenderer::createSemaphores() { semaphore = logicalDevice->createSemaphore(); }
std::shared_ptr<vulkan::Semaphore> GBufferRenderer::render() {
  fence->reset();
  commandBuffer->submit(
      {.waitSemaphores = {}, .signalSemaphores = {*semaphore}, .flags = {}, .fence = *fence, .wait = true});
  return semaphore;
}
const std::shared_ptr<vulkan::Image> &GBufferRenderer::getPosAndMaterialImage() const { return posAndMaterialImage; }
const std::shared_ptr<vulkan::ImageView> &GBufferRenderer::getPosAndMaterialImageView() const {
  return posAndMaterialImageView;
}
const std::shared_ptr<vulkan::Image> &GBufferRenderer::getNormalImage() const { return normalImage; }
const std::shared_ptr<vulkan::ImageView> &GBufferRenderer::getNormalImageView() const { return normalImageView; }

const std::shared_ptr<vulkan::Image> &GBufferRenderer::getDebugImage() const { return debugImage; }
const std::shared_ptr<vulkan::ImageView> &GBufferRenderer::getDebugImageView() const { return debugImageView; }
const std::shared_ptr<vulkan::TextureSampler> &GBufferRenderer::getDebugImageSampler() const {
  return debugImageSampler;
}
void GBufferRenderer::setViewType(GBufferViewType viewType) {
  debugUniformBuffer->mapping().set(static_cast<std::uint32_t>(viewType));
}
}// namespace pf