//
// Created by petr on 9/26/20.
//

#ifndef VOXEL_RENDER_TRIANGLERENDERER_H
#define VOXEL_RENDER_TRIANGLERENDERER_H

#include "VulkanDebugCallbackImpl.h"
#include "logging/loggers.h"
#include "ui/ImGuiGlfwVulkan.h"
#include "utils/common_enums.h"
#include <chaiscript/chaiscript.hpp>
#include <iostream>
#include <pf_common/coroutines/Sequence.h>
#include <pf_glfw_vulkan/concepts/Window.h>
#include <pf_glfw_vulkan/lib_config.h>
#include <pf_glfw_vulkan/vulkan/types.h>
#include <pf_imgui/elements.h>
#include <pf_imgui/serialization.h>
#include <range/v3/view.hpp>
#include <utils/Camera.h>
#include <utils/FPSCounter.h>

using namespace pf::vulkan::literals;

namespace pf {

class TriangleRenderer : VulkanDebugCallbackImpl {
  std::reference_wrapper<toml::table> config;
  Camera camera;

  std::shared_ptr<vulkan::Texture> testTexture;
  std::shared_ptr<vulkan::TextureSampler> testTextureSampler;
  std::shared_ptr<vulkan::ImageView> testTextureView;
 public:
  explicit TriangleRenderer(toml::table &tomlConfig) : config(tomlConfig), camera({0, 0}) {};
  TriangleRenderer(TriangleRenderer &&other) = default;
  TriangleRenderer &operator=(TriangleRenderer &&other) = default;
  TriangleRenderer(const TriangleRenderer &) = delete;
  TriangleRenderer &operator=(const TriangleRenderer &) = delete;

  template<pf::ui::Window Window>
  void init(Window &window) {
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
        .path = "/home/petr/CLionProjects/realistic_voxel_scene_rendering_in_real_time/src/shaders/"
                "triangle.vert",
        .macros = {},
        .replaceMacros = {}});

    fragShader = vkLogicalDevice->createShader(ShaderConfigGlslFile{
        .name = "Triangle frag",
        .type = ShaderType::Fragment,
        .path = "/home/petr/CLionProjects/realistic_voxel_scene_rendering_in_real_time/src/shaders/"
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
        {.queueFamily = vk::QueueFlagBits::eGraphics,
         .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer});

    vkCommandBuffers = vkCommandPool->createCommandBuffers(
        {.level = vk::CommandBufferLevel::ePrimary,
         .count = static_cast<uint32_t>(vkSwapChain->getFrameBuffers().size())});

    std::ranges::generate_n(std::back_inserter(renderSemaphores),
                            vkSwapChain->getFrameBuffers().size(),
                            [&] { return vkLogicalDevice->createSemaphore(); });
    std::ranges::generate_n(std::back_inserter(fences), vkSwapChain->getFrameBuffers().size(), [&] {
      return vkLogicalDevice->createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
    });
    log(spdlog::level::info, APP_TAG, "Initialising Vulkan done.");

    auto imguiConfig = config.get()["ui"].as_table()->contains("imgui") ? *config.get()["ui"]["imgui"].as_table() : toml::table{};
    imgui =
        std::make_unique<ui::ig::ImGuiGlfwVulkan>(vkLogicalDevice, vkRenderPass, vkSurface, vkSwapChain,
                                                  window.getHandle(), ImGuiConfigFlags{}, imguiConfig);
    initUI();

    camera.setScreenWidth(window.getResolution().width);
    camera.setScreenHeight(window.getResolution().height);
    window.setInputIgnorePredicate([this] {
      return imgui->isWindowHovered() || imgui->isKeyboardCaptured();
    });
    camera.registerControls(window);


    window.setMainLoopCallback([&] { render(); });
  }

  template<typename T>
  static std::function<void(T)> makeChaiPrintFnc(ui::ig::Memo &memo) {
    using namespace std::string_literals;
    return [&memo = memo](const T &val) {
      memo.addRecord("<<< "s + fmt::format("{}", val));
      log(spdlog::level::debug, MAIN_TAG, fmt::format("{}", val));
    };
  }

  void initUI();

  void updateCommandBuffer();

  ~TriangleRenderer();

  void render();

 private:


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

  std::unique_ptr<ui::ig::ImGuiGlfwVulkan> imgui;
  FPSCounter fpsCounter;
  ui::ig::FlameGraph *statsFlameGraph;

  bool isMoved = false;
  std::unique_ptr<chaiscript::ChaiScript> chai = std::make_unique<chaiscript::ChaiScript>();
};

}// namespace pf
#endif//VOXEL_RENDER_TRIANGLERENDERER_H
