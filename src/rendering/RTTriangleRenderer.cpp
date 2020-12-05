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
       .extent = vk::Extent3D{vkSwapChain->getExtent(), 1},
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
  throw NotImplementedException("createPipeline not implemented");
}
void RTTriangleRenderer::prepareCommands() {
  throw NotImplementedException("prepareCommands not implemented");
}
void RTTriangleRenderer::createFences() {
  throw NotImplementedException("createFences not implemented");
}
void RTTriangleRenderer::createSemaphores() {
  throw pf::NotImplementedException("not implemented");
}

}// namespace pf