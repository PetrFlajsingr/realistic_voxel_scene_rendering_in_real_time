//
// Created by petr on 12/5/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_RENDERING_RTTRIANGLERENDERER_H
#define REALISTIC_VOXEL_RENDERING_SRC_RENDERING_RTTRIANGLERENDERER_H

#include "VulkanDebugCallbackImpl.h"
#include "logging/loggers.h"
#include "utils/common_enums.h"
#include <pf_glfw_vulkan/concepts/Window.h>
#include <pf_glfw_vulkan/lib_config.h>
#include <pf_glfw_vulkan/vulkan/types.h>
#include <toml++/toml.h>
#include <utils/Camera.h>

using namespace pf::vulkan::literals;

namespace pf {
class RTTriangleRenderer : VulkanDebugCallbackImpl {
 public:
  explicit RTTriangleRenderer(toml::table &tomlConfig);
  RTTriangleRenderer(const RTTriangleRenderer&) = delete;
  RTTriangleRenderer& operator=(const RTTriangleRenderer&) = delete;
  RTTriangleRenderer(RTTriangleRenderer&&) = default;
  RTTriangleRenderer& operator=(RTTriangleRenderer&&) = default;

  template<pf::ui::Window Window>
  void init([[maybe_unused]] Window &window) {
    pf::vulkan::setGlobalLoggerInstance(std::make_shared<GlobalLoggerInterface>("global_vulkan"));
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
  }

  void render();

 private:
  std::reference_wrapper<toml::table> config;
  Camera camera;

  std::shared_ptr<vulkan::Instance> vkInstance;
  std::shared_ptr<vulkan::Surface> vkSurface;
  std::shared_ptr<vulkan::PhysicalDevice> vkDevice;
  std::shared_ptr<vulkan::LogicalDevice> vkLogicalDevice;
  std::shared_ptr<vulkan::SwapChain> vkSwapChain;
  std::shared_ptr<vulkan::RenderPass> vkRenderPass;
  std::shared_ptr<vulkan::GraphicsPipeline> vkGraphicsPipeline;
  std::shared_ptr<vulkan::CommandPool> vkCommandPool;
  std::vector<std::shared_ptr<vulkan::CommandBuffer>> vkCommandBuffers;
};


}

#endif//REALISTIC_VOXEL_RENDERING_SRC_RENDERING_RTTRIANGLERENDERER_H
