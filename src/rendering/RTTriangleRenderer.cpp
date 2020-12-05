//
// Created by petr on 12/5/20.
//

#include "RTTriangleRenderer.h"

namespace pf {
using namespace vulkan;

RTTriangleRenderer::RTTriangleRenderer(toml::table &tomlConfig)
    : config(tomlConfig), camera({0, 0}) {}

void RTTriangleRenderer::render() { throw NotImplementedException("render not implemented"); }

void RTTriangleRenderer::createDevices() {
  vkDevice = vkInstance->selectDevice(
      DefaultDeviceSuitabilityScorer({}, {}, [](const auto &) { return 0; }));
  vkLogicalDevice = vkDevice->createLogicalDevice(
      {.id = "dev1",
       .deviceFeatures = vk::PhysicalDeviceFeatures{},
       .queueTypes = {vk::QueueFlagBits::eCompute, vk::QueueFlagBits::eGraphics},
       .presentQueueEnabled = true,
       .requiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                    VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME},
       .validationLayers = getValidationLayers(),
       .surface = *vkSurface});
}

void RTTriangleRenderer::createRenderTexture() {
  vkRenderImage = vkLogicalDevice->createImage(
      {.imageType = vk::ImageType::e2D,
       .format = vkSwapChain->getFormat(),
       .extent = vk::Extent3D{.width = vkSwapChain->getExtent().width,
                              .height = vkSwapChain->getExtent().height,
                              .depth = 1},
       .mipLevels = 1,
       .arrayLayers = 1,
       .sampleCount = vk::SampleCountFlagBits::e1,
       .tiling = vk::ImageTiling::eOptimal,
       .usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc,
       .sharingQueues = {},
       .layout = vk::ImageLayout::eUndefined});
  vkRenderImageView = vkRenderImage->createImageView(
      vk::ColorSpaceKHR::eSrgbNonlinear, vk::ImageViewType::e2D,
      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
}
void RTTriangleRenderer::createBuffers() {
  throw NotImplementedException("createBuffers not implemented");
}
void RTTriangleRenderer::createDescriptorPool() {
  vkDescPool = vkLogicalDevice->createDescriptorPool(
      {.flags = {}, .maxSets = 1, .poolSizes = {{vk::DescriptorType::eStorageImage, 3}}});
}

void RTTriangleRenderer::createPipeline() {
  // TODO: compute pipeline builder
  // TODO: descriptor sets
  vkComputeDescSetLayout = vkLogicalDevice->createDescriptorSetLayout(
      {.bindings = {{.binding = 0,
                     .type = vk::DescriptorType::eStorageImage,
                     .count = 1,
                     .stageFlags = vk::ShaderStageFlagBits::eCompute}}});
  const auto setLayouts = std::vector{**vkComputeDescSetLayout};
  const auto pipelineLayoutInfo =
      vk::PipelineLayoutCreateInfo{.setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
                                   .pSetLayouts = setLayouts.data()};
  auto computePipelineLayout = (*vkLogicalDevice)->createPipelineLayoutUnique(pipelineLayoutInfo);
  auto allocInfo = vk::DescriptorSetAllocateInfo{};
  allocInfo.setSetLayouts(setLayouts);
  computeDescriptorSets = (*vkLogicalDevice)->allocateDescriptorSetsUnique(allocInfo);

  const auto computeInfo = vk::DescriptorImageInfo{.sampler = {},
                                                   .imageView = **vkRenderImageView,
                                                   .imageLayout = vk::ImageLayout::eGeneral};
  const auto imageInfo = std::vector{computeInfo};
  const auto computeWrite =
      vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                             .dstBinding = 0,
                             .dstArrayElement = {},
                             .descriptorCount = 1,
                             .descriptorType = vk::DescriptorType::eStorageImage,
                             .pImageInfo = imageInfo.data()};

  const auto writeSets = std::vector{computeWrite};
  (*vkLogicalDevice)->updateDescriptorSets(writeSets, nullptr);

  auto computeShader = vkLogicalDevice->createShader(ShaderConfigGlslFile{
      .name = "Triangle compute",
      .type = ShaderType::Compute,
      .path = "/home/petr/CLionProjects/realistic_voxel_scene_rendering_in_real_time/src/shaders/"
              "rt_triangle.comp",
      .macros = {},
      .replaceMacros = {}});

  const auto computeStageInfo =
      vk::PipelineShaderStageCreateInfo{.stage = computeShader->getVkType(), .pName = "main"};
  const auto pipelineInfo =
      vk::ComputePipelineCreateInfo{.stage = computeStageInfo, .layout = *computePipelineLayout};
  vkComputePipeline = ComputePipeline::CreateShared(
      (*vkLogicalDevice)->createComputePipelineUnique(nullptr, pipelineInfo).value,
      std::move(computePipelineLayout));
}
void RTTriangleRenderer::createCommands() {
  vkCommandPool = vkLogicalDevice->createCommandPool(
      {.queueFamily = vk::QueueFlagBits::eCompute,
       .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer});

  vkCommandBuffers =
      vkCommandPool->createCommandBuffers({.level = vk::CommandBufferLevel::ePrimary, .count = 1});
}
void RTTriangleRenderer::recordCommands() {
  // TODO: add these calls to recording
  auto &buffer = vkCommandBuffers[0];
  auto recording = buffer->begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
  recording.bindPipeline(vk::PipelineBindPoint::eCompute, *vkComputePipeline);

  const auto vkDescSets = computeDescriptorSets
      | ranges::views::transform([](auto &descSet) { return *descSet; }) | ranges::to_vector;
  recording.getCommandBuffer()->bindDescriptorSets(
      vk::PipelineBindPoint::eCompute, vkComputePipeline->getVkPipelineLayout(), 0, vkDescSets, {});
  recording.dispatch(vkSwapChain->getExtent().width, vkSwapChain->getExtent().height, 1);
  {
    auto imageBarriers = std::vector<vk::ImageMemoryBarrier>{};
    imageBarriers.emplace_back(vkRenderImage->createImageBarrier(
        {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}, {}, vk::AccessFlagBits::eShaderWrite,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral));
    imageBarriers.emplace_back(vkRenderImage->createImageBarrier(
        {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}, vk::AccessFlagBits::eShaderWrite,
        vk::AccessFlagBits::eTransferRead, vk::ImageLayout::eGeneral,
        vk::ImageLayout::eTransferSrcOptimal));
    imageBarriers.emplace_back(vkRenderImage->createImageBarrier(
        {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}, {}, vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal));
    recording.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                              vk::PipelineStageFlagBits::eAllCommands, {}, {}, imageBarriers);
  }

  auto &currentSwapchainImage = *vkSwapChain->getImages()[vkSwapChain->getCurrentImageIndex()];
  recording.copyImage({.src = *vkRenderImage,
                       .dst = currentSwapchainImage,
                       .srcLayout = vk::ImageLayout::eTransferSrcOptimal,
                       .dstLayout = vk::ImageLayout::eTransferDstOptimal,
                       .srcLayers = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                                     .mipLevel = 0,
                                     .baseArrayLayer = 0,
                                     .layerCount = 1},
                       .dstLayers = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                                     .mipLevel = 0,
                                     .baseArrayLayer = 0,
                                     .layerCount = 1},
                       .srcOffset = {0, 0, 0},
                       .dstOffset = {0, 0, 0}});

  {
    auto imageBarriers = std::vector<vk::ImageMemoryBarrier>{};
    imageBarriers.emplace_back(currentSwapchainImage.createImageBarrier(
        {.aspectMask = vk::ImageAspectFlagBits::eColor,
         .baseMipLevel = 0,
         .levelCount = 1,
         .baseArrayLayer = 0,
         .layerCount = 1},
        vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eMemoryRead,
        vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR));
    recording.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
                              vk::PipelineStageFlagBits::eTopOfPipe, {}, {}, imageBarriers);
  }
}
void RTTriangleRenderer::createFences() {
  throw NotImplementedException("createFences not implemented");
}
void RTTriangleRenderer::createSemaphores() {
  throw pf::NotImplementedException("not implemented");
}

}// namespace pf