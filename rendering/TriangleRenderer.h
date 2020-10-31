//
// Created by petr on 9/26/20.
//

#ifndef VOXEL_RENDER_TRIANGLERENDERER_H
#define VOXEL_RENDER_TRIANGLERENDERER_H

#include "../concepts/Window.h"
#include "../coroutines/Sequence.h"
#include "../logging/loggers.h"
#include "../ui/imgui/ImGuiGlfwVulkan.h"
#include "../utils/common_enums.h"
#include "../vulkan/types/builders/GraphicsPipelineBuilder.h"
#include "../vulkan/types/builders/RenderPassBuilder.h"
#include "../vulkan/types/types.h"
#include "imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#include <range/v3/view.hpp>

using namespace pf::vulkan::literals;

namespace pf {
class TriangleRenderer {
 public:
  template<pf::ui::Window Window>
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
                      .dependency()
                        .srcSubpass()
                        .dstSubpass("main")
                        .srcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                        .dstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                        .dstAccessFlags(vk::AccessFlagBits::eColorAttachmentWrite)
                      .dependencyDone()
                    .subpassDone()
                    .build();
    // clang-format on
    vertShader = vkLogicalDevice->createShader(ShaderConfigGlslFile{
        .name = "Triangle vert",
        .type = ShaderType::Vertex,
        .path = "/home/petr/CLionProjects/realistic_voxel_scene_rendering_in_real_time/shaders/"
                "triangle.vert",
        .macros = {},
        .replaceMacros = {}});

    fragShader = vkLogicalDevice->createShader(ShaderConfigGlslFile{
        .name = "Triangle frag",
        .type = ShaderType::Fragment,
        .path = "/home/petr/CLionProjects/realistic_voxel_scene_rendering_in_real_time/shaders/"
                "triangle.frag",
        .macros = {},
        .replaceMacros = {}});
    // clang-format off
    vkGraphicsPipeline = GraphicsPipelineBuilder()
                    .shader(*vertShader, "main")
                    .shader(*fragShader, "main")
                    .topology(vk::PrimitiveTopology::eTriangleList)
                    .primitiveRestart(Enabled::No)
                    .viewport({0.f,
                                   0.f,
                                   static_cast<float>(vkSwapChain->getExtent().width),
                                   static_cast<float>(vkSwapChain->getExtent().height),
                                   0.f,
                                   1.f})
                    .scissor({{0, 0},
                                  vkSwapChain->getExtent()})
                    .depthClamp(Enabled::No)
                    .rastDiscard(Enabled::No)
                    .polygonMode(vk::PolygonMode::eFill)
                    .lineWidth(1.f)
                    .cullMode(vk::CullModeFlagBits::eBack)
                    .frontFace(vk::FrontFace::eClockwise)
                    .depthBias(Enabled::No)
                    .setMsSampleShading(Enabled::No)
                    .rasterizationSamples(vk::SampleCountFlagBits::e1)
                    .blend(Enabled::No)
                    .blendColorMask(vk::ColorComponentFlagBits::eR
                                    | vk::ColorComponentFlagBits::eG
                                    | vk::ColorComponentFlagBits::eB
                                    | vk::ColorComponentFlagBits::eA)
                    .blendLogicOpEnabled(Enabled::No)
                    .blendLogicOp(vk::LogicOp::eCopy)
                    .blendConstants({{0.f, 0.f, 0.f, 0.f}})
                    .build(vkRenderPass);
    // clang-format on

    vkCommandPool = vkLogicalDevice->createCommandPool(
        {.queueFamily = vk::QueueFlagBits::eGraphics, .flags = {}});

    vkCommandBuffers = vkCommandPool->createCommandBuffers(
        {.level = vk::CommandBufferLevel::ePrimary,
         .count = static_cast<uint32_t>(vkSwapChain->getFrameBuffers().size())});

    for (std::weakly_incrementable auto i : std::views::iota(0ul, vkCommandBuffers.size())) {
      auto clearValues = std::vector<vk::ClearValue>(2);
      clearValues[0].setColor({std::array<float, 4>{0.f, 1.f, 0.f, 0.f}});
      clearValues[1].setDepthStencil({1.f, 0});
      vkCommandBuffers[i]
          ->begin(vk::CommandBufferUsageFlagBits::eRenderPassContinue)
          .beginRenderPass({.renderPass = *vkRenderPass,
                            .frameBuffer = *vkSwapChain->getFrameBuffers()[i],
                            .clearValues = clearValues,
                            .extent = vkSwapChain->getExtent()})
          .bindPipeline(vk::PipelineBindPoint::eGraphics, *vkGraphicsPipeline)
          .draw({3, 1, 0, 0})
          .endRenderPass();
    }

    std::ranges::generate_n(std::back_inserter(renderSemaphores),
                            vkSwapChain->getFrameBuffers().size(),
                            [&] { return vkLogicalDevice->createSemaphore(); });
    std::ranges::generate_n(std::back_inserter(fences), vkSwapChain->getFrameBuffers().size(), [&] {
      return vkLogicalDevice->createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
    });

    log(spdlog::level::info, APP_TAG, "Initialising Vulkan done.");

    window.setMainLoopCallback([&] { render(); });
  }

  void render() {
    vkSwapChain->swap();
    auto &semaphore = vkSwapChain->getCurrentSemaphore();
    auto &fence = vkSwapChain->getCurrentFence();
    const auto commandBufferIndex = vkSwapChain->getCurrentImageIndex();
    const auto frameIndex = vkSwapChain->getCurrentFrameIndex();

    fence.reset();
    vkCommandBuffers[commandBufferIndex]->submit(
        {.waitSemaphores = {semaphore},
         .signalSemaphores = {*renderSemaphores[frameIndex]},
         .flags = vk::PipelineStageFlagBits::eColorAttachmentOutput,
         .fence = fence,
         .wait = false});

    vkSwapChain->present(vulkan::PresentConfig{.waitSemaphores = {*renderSemaphores[frameIndex]},
                                               .presentQueue = vkLogicalDevice->getPresentQueue()});
    vkSwapChain->frameDone();
  }

 private:
  static bool debugCallback(const vulkan::DebugCallbackData &data,
                            vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                            const vk::DebugUtilsMessageTypeFlagsEXT &);

  vk::Format getDepthFormat();

  std::shared_ptr<vulkan::Instance> vkInstance;
  std::shared_ptr<vulkan::Surface> vkSurface;
  std::shared_ptr<vulkan::PhysicalDevice> vkDevice;
  std::shared_ptr<vulkan::LogicalDevice> vkLogicalDevice;
  std::shared_ptr<vulkan::SwapChain> vkSwapChain;
  std::shared_ptr<vulkan::RenderPass> vkRenderPass;
  std::shared_ptr<vulkan::GraphicsPipeline> vkGraphicsPipeline;
  std::shared_ptr<vulkan::CommandPool> vkCommandPool;
  std::vector<std::shared_ptr<vulkan::CommandBuffer>> vkCommandBuffers;

  std::vector<std::shared_ptr<vulkan::Semaphore>> renderSemaphores;
  std::vector<std::shared_ptr<vulkan::Fence>> fences;

  std::shared_ptr<vulkan::Shader> vertShader;
  std::shared_ptr<vulkan::Shader> fragShader;
};

}// namespace pf
#endif//VOXEL_RENDER_TRIANGLERENDERER_H
