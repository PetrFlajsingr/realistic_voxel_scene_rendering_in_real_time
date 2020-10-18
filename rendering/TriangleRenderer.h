//
// Created by petr on 9/26/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_TRIANGLERENDERER_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_TRIANGLERENDERER_H

#include "../concepts/Window.h"
#include "../logging/loggers.h"
#include "../vulkan/types/Instance.h"
#include "../vulkan/types/PhysicalDevice.h"
#include "../vulkan/types/RenderPass.h"
#include "../vulkan/types/Surface.h"
#include "../vulkan/types/SwapChain.h"
#include "../vulkan/types/VulkanCommon.h"
#include "../vulkan/types/builders/RenderPassBuilder.h"
#include <range/v3/view.hpp>

using namespace pf::vulkan::literals;

namespace pf {
class TriangleRenderer {
 public:
  template<pf::window::Window Window>
  void init(Window &window) {
    using namespace vulkan;
    log(spdlog::level::info, APP_TAG, "Initialising Vulkan.");
    using namespace ranges;
    const auto windowExtensions = window.requiredVulkanExtensions();
    auto validationLayers = std::unordered_set<std::string>{"VK_LAYER_KHRONOS_validation"};
    vkInstance = Instance::CreateShared(
        InstanceConfig{.appName = "Realistic voxel rendering in real time",
                       .appVersion = "0.1.0"_v,
                       .vkVersion = "1.2.0"_v,
                       .engineInfo = EngineInfo{.name = "<unnamed>", .engineVersion = "0.1.0"_v},
                       .requiredWindowExtensions = windowExtensions,
                       .validationLayers = validationLayers,
                       .callback = [this](const DebugCallbackData &data,
                                          vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                          const vk::DebugUtilsMessageTypeFlagsEXT &type_flags) {
                         return debugCallback(data, severity, type_flags);
                       }});
    vkSurface = Surface::CreateShared(vkInstance, window);
    vkDevice = vkInstance->selectDevice(
        DefaultDeviceSuitabilityScorer({}, {}, [](const auto &) { return 0; }));
    vkLogicalDevice = vkDevice->createLogicalDevice(
        {.id = "dev1",
         .deviceFeatures = vk::PhysicalDeviceFeatures{},
         .queueTypes = {vk::QueueFlagBits::eCompute, vk::QueueFlagBits::eGraphics},
         .presentQueueEnabled = true,
         .requiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                      VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME},
         .validationLayers = validationLayers,
         .surface = *vkSurface});
    auto queuesView = vkLogicalDevice->getQueueIndices() | views::values;
    auto sharingQueues = std::unordered_set(queuesView.begin(), queuesView.end());
    if (const auto presentIdx = vkLogicalDevice->getPresentQueueIndex(); presentIdx.has_value()) {
      sharingQueues.emplace(*presentIdx);
    }
    vkSwapChain = vkLogicalDevice->createSwapChain(
        vkSurface,
        {.formats = {{vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear}},
         .presentModes = {vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo},
         .resolution = {window.getResolution().width, window.getResolution().height},
         .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
         .sharingQueues = sharingQueues,
         .imageArrayLayers = 1,
         .clipped = true,
         .oldSwapChain = std::nullopt,
         .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque});

    // clang-format off
    vkRenderPass = RenderPassBuilder(vkLogicalDevice)
                    .attachment("color")
                       .format(vkSwapChain->getFormat())
                       .samples(vk::SampleCountFlagBits::e2)
                       .loadOp(vk::AttachmentLoadOp::eClear)
                       .storeOp(vk::AttachmentStoreOp::eStore)
                       .stencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                       .stencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                       .initialLayout(vk::ImageLayout::eUndefined)
                       .finalLayout(vk::ImageLayout::eColorAttachmentOptimal)
                    .attachmentDone()
                    .attachment("depth")
                      .format(getDepthFormat())
                      .samples(vk::SampleCountFlagBits::e2)
                      .loadOp(vk::AttachmentLoadOp::eClear)
                      .storeOp(vk::AttachmentStoreOp::eDontCare)
                      .stencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                      .stencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                      .initialLayout(vk::ImageLayout::eUndefined)
                      .finalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                    .attachmentDone()
                    .attachment("color_resolve")
                      .format(vkSwapChain->getFormat())
                      .samples(vk::SampleCountFlagBits::e1)
                      .loadOp(vk::AttachmentLoadOp::eClear)
                      .storeOp(vk::AttachmentStoreOp::eStore)
                      .stencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                      .stencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                      .initialLayout(vk::ImageLayout::eUndefined)
                      .finalLayout(vk::ImageLayout::ePresentSrcKHR)
                    .attachmentDone()
                    .subpass("main")
                      .pipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                      .colorAttachment("color")
                      .colorAttachment("depth")
                      .colorAttachment("color_resolve")
                      .dependency()
                        .srcSubpass()
                        .dstSubpass("main")
                        .srcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                        .dstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                        .dstAccessFlags(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
                      .dependencyDone()
                    .subpassDone()
                    .build();
    // clang-format on
    log(spdlog::level::info, APP_TAG, "Initialising Vulkan done.");
  }

  void render() {}

 private:
  bool debugCallback(const vulkan::DebugCallbackData &data,
                     vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                     const vk::DebugUtilsMessageTypeFlagsEXT &);

  vk::Format getDepthFormat();

  std::shared_ptr<vulkan::Instance> vkInstance;
  std::shared_ptr<vulkan::Surface> vkSurface;
  std::shared_ptr<vulkan::PhysicalDevice> vkDevice;
  std::shared_ptr<vulkan::LogicalDevice> vkLogicalDevice;
  std::shared_ptr<vulkan::SwapChain> vkSwapChain;
  std::shared_ptr<vulkan::RenderPass> vkRenderPass;
};

}// namespace pf
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_TRIANGLERENDERER_H
