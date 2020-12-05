//
// Created by petr on 12/5/20.
//

#include "RTTriangleRenderer.h"

namespace pf {

RTTriangleRenderer::RTTriangleRenderer(toml::table &tomlConfig)
    : config(tomlConfig), camera({0, 0}) {}

void RTTriangleRenderer::render() {
  throw NotImplementedException("render not implemented");
}

void RTTriangleRenderer::createDevices() {
  using namespace vulkan;
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
  throw NotImplementedException("createRenderTexture not implemented");
}
void RTTriangleRenderer::createBuffers() {
  throw NotImplementedException("createBuffers not implemented");
}
void RTTriangleRenderer::createDescriptorPool() {
  throw NotImplementedException("createDescriptorPool not implemented");
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

}