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
#include <range/v3/view/map.hpp>

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
  void init(Window &window) {
    pf::vulkan::setGlobalLoggerInstance(std::make_shared<GlobalLoggerInterface>("global_vulkan"));
    log(spdlog::level::info, APP_TAG, "Initialising Vulkan.");

    createInstance(window);
    createSurface(window);
    createDevices();

    createSwapchain();
    createRenderTexture();
    createBuffers();
    createDescriptorPool();
    createPipeline();

    prepareCommands();
    createFences();
    createSemaphores();
  }

  void render();

 private:
  std::unordered_set<std::string> getValidationLayers() {
    return std::unordered_set<std::string>{"VK_LAYER_KHRONOS_validation"};
  }
  template<pf::ui::Window Window>
  void createInstance(Window &window) {
    using namespace vulkan;
    const auto windowExtensions = window.requiredVulkanExtensions();
    auto validationLayers = getValidationLayers();
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
  }

  template<pf::ui::Window Window>
  void createSurface(Window &window) {
    using namespace vulkan;
    vkSurface = Surface::CreateShared(vkInstance, window);
  }
  void createDevices();

  template<pf::ui::Window Window>
  void createSwapchain(Window &window) {
    using namespace ranges;
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
  }
  void createRenderTexture();
  void createBuffers();
  void createDescriptorPool();
  void createPipeline();
  void prepareCommands();
  void createFences();
  void createSemaphores();

  std::reference_wrapper<toml::table> config;
  Camera camera;

  std::shared_ptr<vulkan::Instance> vkInstance;
  std::shared_ptr<vulkan::Surface> vkSurface;
  std::shared_ptr<vulkan::PhysicalDevice> vkDevice;
  std::shared_ptr<vulkan::LogicalDevice> vkLogicalDevice;
  std::shared_ptr<vulkan::SwapChain> vkSwapChain;
  std::shared_ptr<vulkan::DescriptorPool> vkDescPool;
  std::shared_ptr<vulkan::RenderPass> vkRenderPass;
  std::shared_ptr<vulkan::GraphicsPipeline> vkGraphicsPipeline;
  std::shared_ptr<vulkan::CommandPool> vkCommandPool;
  std::vector<std::shared_ptr<vulkan::CommandBuffer>> vkCommandBuffers;

  std::shared_ptr<vulkan::Image> vkRenderImage;
  std::shared_ptr<vulkan::ImageView> vkRenderImageView;
};


}

#endif//REALISTIC_VOXEL_RENDERING_SRC_RENDERING_RTTRIANGLERENDERER_H
