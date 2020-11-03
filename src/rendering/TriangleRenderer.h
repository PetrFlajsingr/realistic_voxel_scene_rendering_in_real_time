//
// Created by petr on 9/26/20.
//

#ifndef VOXEL_RENDER_TRIANGLERENDERER_H
#define VOXEL_RENDER_TRIANGLERENDERER_H

#include "concepts/Window.h"
#include <pf_common/coroutines/Sequence.h>
#include "logging/loggers.h"
#include "ui/imgui/ImGuiGlfwVulkan.h"
#include "ui/imgui/elements.h"
#include "ui/imgui/serialization.h"
#include <pf_common/Visitor.h>
#include "utils/common_enums.h"
#include "vulkan/types/builders/GraphicsPipelineBuilder.h"
#include "vulkan/types/builders/RenderPassBuilder.h"
#include "vulkan/types/types.h"
#include "imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#include <chaiscript/chaiscript.hpp>
#include <iostream>
#include <range/v3/view.hpp>

using namespace pf::vulkan::literals;

namespace pf {
class TriangleRenderer {
  std::reference_wrapper<TomlConfig> config;

 public:
  explicit TriangleRenderer(TomlConfig &tomlConfig) : config(tomlConfig){};
  TriangleRenderer(TriangleRenderer &&other) = default;
  TriangleRenderer &operator=(TriangleRenderer &&other) = default;
  TriangleRenderer(const TriangleRenderer &) = delete;
  TriangleRenderer &operator=(const TriangleRenderer &) = delete;

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

    auto imguiConfig = config.get()["ui"].as_table()->contains("imgui") ? *config.get()["ui"]["imgui"].as_table() : toml::table{};
    imgui =
        std::make_unique<ui::ig::ImGuiGlfwVulkan>(vkLogicalDevice, vkRenderPass, vkSurface, vkSwapChain,
                                              window.getHandle(), ImGuiConfigFlags{}, imguiConfig);
    initUI();

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

    window.setMainLoopCallback([&] { render(); });
  }

  template<typename T>
  static std::function<void(T)> makeChaiPrintFnc(const std::shared_ptr<ui::ig::Memo> &memo) {
    using namespace std::string_literals;
    return [memo](const T &val) {
      memo->addRecord("<<< "s + fmt::format("{}", val));
      log(spdlog::level::debug, MAIN_TAG, fmt::format("{}", val));
    };
  }

  void initUI() {
    using namespace std::string_literals;
    using namespace pf::ui::ig;
    auto logWindow = imgui->createChild<Window>("log_window", "Log");
    logWindow->createChild<Checkbox>("chkbx", "test check")->addValueListener([](auto a) {
      std::cout << std::boolalpha << a << std::endl;
    });
    auto logMemo = logWindow->createChild<Memo>("log_output", "Log:", 100, true, true, 100);
    addLogListener([logMemo](auto record) { logMemo->addRecord(record); });
    auto logErrMemo =
        logWindow->createChild<Memo>("log_err_output", "Log err:", 100, true, true, 100);
    addLogListener([logErrMemo](auto record) { logErrMemo->addRecord(record); }, true);

    auto chaiWindow = imgui->createChild<Window>("chai_window", "ChaiScript");
    auto chaiInputPanel = chaiWindow->createChild<Panel>("chai_input_panel", "Input",
                                                         PanelLayout::Horizontal, ImVec2{0, 50});

    chaiInputPanel->createChild<Text>("chain_input_label", "Input:");
    auto chaiInput =
        chaiInputPanel->createChild<InputText>("chai_input", "", "", TextInputType::MultiLine);
    auto chai_output =
        chaiWindow->createChild<Memo>("chai_output", "Output:", 100, true, true, 100);

    chai->add(chaiscript::fun(makeChaiPrintFnc<int>(chai_output)), "print");
    chai->add(chaiscript::fun(makeChaiPrintFnc<double>(chai_output)), "print");
    chai->add(chaiscript::fun(makeChaiPrintFnc<float>(chai_output)), "print");
    chai->add(chaiscript::fun(makeChaiPrintFnc<std::string>(chai_output)), "print");
    chaiInputPanel->createChild<Button>("chain_input_confirm", "Confirm")
        ->setOnClick([chaiInput, chai_output, this] {
          const auto input = chaiInput->getText();
          chai_output->addRecord(">>> "s + input);
          chaiInput->clear();
          try {
            chai->eval(input);
          } catch (const chaiscript::exception::eval_error &e) {
            chai_output->addRecord("<<< "s + e.pretty_print());
          }
        });

    chai->add(
        chaiscript::fun([](const std::string &str) { log(spdlog::level::debug, APP_TAG, str); }),
        "log");

    auto testWindow = imgui->createChild<Window>("test window", "TEST");
    testWindow->createChild<ColorChooser<ColorChooserType::Picker, glm::vec4>>("col1", "col1",
                                                                               Persistent::Yes);
    testWindow->createChild<ColorChooser<ColorChooserType::Edit, glm::vec3>>("col2", "col2",
                                                                             Persistent::Yes);
    testWindow
        ->createChild<ComboBox>("cb1", "cb1", "preview", std::vector{"1"s, "2"s, "3"s},
                                Persistent::Yes)
        ->addValueListener([](auto str) { logdFmt(MAIN_TAG, "cb1: {}", str); });

    testWindow->createChild<Input<glm::vec2>>("input1", "input1", Persistent::Yes)
        ->addValueListener([](auto val) { logdFmt(MAIN_TAG, "input1 :{}x{}", val.x, val.y); });
    testWindow
        ->createChild<InputText>("input2", "input2", "", TextInputType::SingleLine, Persistent::Yes)
        ->addValueListener([](auto val) { logdFmt(MAIN_TAG, "input2 :{}", val); });

    auto rgroup = testWindow->createChild<RadioGroup>("rgroup", "group", std::vector<RadioButton>{},
                                                      std::nullopt, Persistent::Yes);
    rgroup->addButton("rb1", "1");
    rgroup->addButton("rb2", "2");
    rgroup->addButton("rb3", "3");
    rgroup->addButton("rb4", "4");

    testWindow->createChild<Slider<glm::vec2>>("Slider1", "Slider1", -100.f, 100, glm::vec2{}, Persistent::Yes);
    testWindow->createChild<Slider<int>>("Slider2", "Slide2", -100, 100, 0, Persistent::Yes);

    imgui->setStateFromConfig();
  }

  void updateCommandBuffer() {
    for (std::weakly_incrementable auto i : std::views::iota(0ul, vkCommandBuffers.size())) {
      auto clearValues = std::vector<vk::ClearValue>(2);
      clearValues[0].setColor({std::array<float, 4>{0.f, 1.f, 0.f, 0.f}});
      clearValues[1].setDepthStencil({1.f, 0});
      auto recorder =
          vkCommandBuffers[i]->begin(vk::CommandBufferUsageFlagBits::eRenderPassContinue);
      recorder
          .beginRenderPass({.renderPass = *vkRenderPass,
                            .frameBuffer = *vkSwapChain->getFrameBuffers()[i],
                            .clearValues = clearValues,
                            .extent = vkSwapChain->getExtent()})
          .bindPipeline(vk::PipelineBindPoint::eGraphics, *vkGraphicsPipeline)
          .draw({3, 1, 0, 0});
      imgui->addToCommandBuffer(recorder);

      recorder.endRenderPass();
    }
  }

  ~TriangleRenderer();

  void render() {
    vkSwapChain->swap();

    imgui->render();

    updateCommandBuffer();

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
         .wait = true});

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

  std::unique_ptr<ui::ig::ImGuiInterface> imgui;

  bool isMoved = false;
  std::unique_ptr<chaiscript::ChaiScript> chai = std::make_unique<chaiscript::ChaiScript>();
};

}// namespace pf
#endif//VOXEL_RENDER_TRIANGLERENDERER_H
