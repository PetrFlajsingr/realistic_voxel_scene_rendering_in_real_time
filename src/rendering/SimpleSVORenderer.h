//
// Created by petr on 1/5/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_RENDERING_SIMPLESVORENDERER_H
#define REALISTIC_VOXEL_RENDERING_SRC_RENDERING_SIMPLESVORENDERER_H

#include "VulkanDebugCallbackImpl.h"
#include "logging/loggers.h"
#include "ui/ImGuiGlfwVulkan.h"
#include "utils/common_enums.h"
#include <RunInfo.h>
#include <chaiscript/chaiscript.hpp>
#include <pf_glfw_vulkan/concepts/Window.h>
#include <pf_glfw_vulkan/lib_config.h>
#include <pf_glfw_vulkan/ui/events/common.h>
#include <pf_glfw_vulkan/vulkan/types.h>
#include <range/v3/view/map.hpp>
#include <toml++/toml.h>
#include <ui/SimpleSVORenderer_UI.h>
#include <utils/Camera.h>
#include <utils/FPSCounter.h>
#include <voxel/SparseVoxelOctree.h>

namespace pf {

constexpr auto LOCAL_SIZE_X = 8;
constexpr auto LOCAL_SIZE_Y = 8;
class SimpleSVORenderer : public VulkanDebugCallbackImpl {
 public:
  explicit SimpleSVORenderer(toml::table &tomlConfig);
  SimpleSVORenderer(const SimpleSVORenderer &) = delete;
  SimpleSVORenderer &operator=(const SimpleSVORenderer &) = delete;
  SimpleSVORenderer(SimpleSVORenderer &&) = default;
  SimpleSVORenderer &operator=(SimpleSVORenderer &&) = default;
  virtual ~SimpleSVORenderer();

  template<pf::ui::Window Window>
  void init(Window &window) {
    closeWindow = [&window] { window.close(); };
    camera.setSwapLeftRight(false);
    pf::vulkan::setGlobalLoggerInstance(std::make_shared<GlobalLoggerInterface>("global_vulkan"));
    log(spdlog::level::info, APP_TAG, "Initialising Vulkan.");

    createInstance(window);
    createSurface(window);
    createDevices();

    buildVulkanObjects(window);

    auto imguiConfig =
        config.get()["ui"].as_table()->contains("imgui") ? *config.get()["ui"]["imgui"].as_table() : toml::table{};
    auto imgui = std::make_unique<ui::ig::ImGuiGlfwVulkan>(vkLogicalDevice, vkRenderPass, vkSurface, vkSwapChain,
                                                           window.getHandle(), ImGuiConfigFlags{}, imguiConfig);
    window.addKeyListener(events::KeyEventType::Pressed, [this](const events::KeyEvent &event) {
      if (event.key == 'H') {
        switch (ui->imgui->getVisibility()) {
          case ui::ig::Visibility::Visible: ui->imgui->setVisibility(ui::ig::Visibility::Invisible); break;
          case ui::ig::Visibility::Invisible: ui->imgui->setVisibility(ui::ig::Visibility::Visible); break;
        }
        return true;
      }
      return false;
    });

    camera.setScreenWidth(window.getResolution().width);
    camera.setScreenHeight(window.getResolution().height);
    window.setInputIgnorePredicate([this] { return ui->imgui->isWindowHovered() || ui->imgui->isKeyboardCaptured(); });
    camera.registerControls(window);

    ui = std::make_unique<SimpleSVORenderer_UI>(std::move(imgui), camera,
                                                TextureData{*vkIterImage, *vkIterImageView, *vkIterImageSampler});

    initUI();
    window.setMainLoopCallback([&] { render(); });
  }

  void render();
  void stop();

 private:
  static std::unordered_set<std::string> getValidationLayers() {
    return std::unordered_set<std::string>{"VK_LAYER_KHRONOS_validation"};
  }
  template<pf::ui::Window Window>
  void buildVulkanObjects(Window &window) {
    createSwapchain(window);
    createRenderTextures();
    createDescriptorPool();
    createPipeline();

    createCommands();
    createFences();
    createSemaphores();

    // clang-format off
    vkRenderPass = vulkan::RenderPassBuilder(vkLogicalDevice)
        .attachment("color")
        .format(vkSwapChain->getFormat())
        .samples(vk::SampleCountFlagBits::e1)
        .loadOp(vk::AttachmentLoadOp::eDontCare)
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
  }

  template<pf::ui::Window Window>
  void createInstance(Window &window) {
    using namespace vulkan;
    using namespace vulkan::literals;
    const auto windowExtensions = window.requiredVulkanExtensions();
    auto validationLayers = getValidationLayers();
    vkInstance = Instance::CreateShared(InstanceConfig{
        .appName = "Realistic voxel rendering in real time",
        .appVersion = "0.1.0"_v,
        .vkVersion = "1.2.0"_v,
        .engineInfo = EngineInfo{.name = "<unnamed>", .engineVersion = "0.1.0"_v},
        .requiredWindowExtensions = windowExtensions,
        .validationLayers = validationLayers,
        .callback = [this](const DebugCallbackData &data, vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
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
         //.presentModes = {vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo},
         .presentModes = {vk::PresentModeKHR::eImmediate},
         .resolution = {window.getResolution().width, window.getResolution().height},
         .imageUsage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst
             | vk::ImageUsageFlagBits::eColorAttachment,
         .sharingQueues = {},
         .imageArrayLayers = 1,
         .clipped = true,
         .oldSwapChain = std::nullopt,
         .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque});
  }
  void createRenderTextures();
  void createDescriptorPool();
  void createPipeline();
  void createCommands();
  void recordCommands();
  void createFences();
  void createSemaphores();

  void initUI();

  void updateTransformMatrix();

  std::vector<std::string> loadModelFileNames(const std::filesystem::path &dir);

  std::reference_wrapper<toml::table> config;
  Camera camera;

  std::shared_ptr<vulkan::DescriptorPool> vkDescPool;
  std::vector<vk::UniqueDescriptorSet> computeDescriptorSets;
  std::shared_ptr<vulkan::Instance> vkInstance;
  std::shared_ptr<vulkan::Surface> vkSurface;
  std::shared_ptr<vulkan::PhysicalDevice> vkDevice;
  std::shared_ptr<vulkan::LogicalDevice> vkLogicalDevice;
  std::shared_ptr<vulkan::SwapChain> vkSwapChain;
  std::shared_ptr<vulkan::CommandPool> vkCommandPool;
  std::shared_ptr<vulkan::CommandPool> vkGraphicsCommandPool;
  std::vector<std::shared_ptr<vulkan::CommandBuffer>> vkCommandBuffers;
  std::vector<std::shared_ptr<vulkan::CommandBuffer>> vkGraphicsCommandBuffers;

  std::shared_ptr<vulkan::Image> vkRenderImage;
  std::shared_ptr<vulkan::ImageView> vkRenderImageView;
  std::shared_ptr<vulkan::Image> vkIterImage;
  std::shared_ptr<vulkan::ImageView> vkIterImageView;
  std::shared_ptr<vulkan::TextureSampler> vkIterImageSampler;

  std::shared_ptr<vulkan::DescriptorSetLayout> vkComputeDescSetLayout;

  std::shared_ptr<vulkan::Buffer> cameraUniformBuffer;
  std::shared_ptr<vulkan::Buffer> lightUniformBuffer;
  std::shared_ptr<vulkan::Buffer> debugUniformBuffer;
  std::shared_ptr<vulkan::Buffer> svoBuffer;
  std::shared_ptr<vulkan::Semaphore> computeSemaphore;
  std::vector<std::shared_ptr<vulkan::Semaphore>> renderSemaphores;

  std::vector<std::shared_ptr<vulkan::Fence>> fences;
  std::shared_ptr<vulkan::ComputePipeline> vkComputePipeline;
  std::shared_ptr<vulkan::Fence> vkComputeFence;
  std::shared_ptr<vulkan::RenderPass> vkRenderPass;

  std::unique_ptr<SimpleSVORenderer_UI> ui;

  FPSCounter fpsCounter;

  std::unique_ptr<chaiscript::ChaiScript> chai = std::make_unique<chaiscript::ChaiScript>();

  std::unique_ptr<vox::SparseVoxelOctree> svo = nullptr;

  bool isSceneLoaded = true;

  std::vector<Subscription> subscriptions;

  std::function<void()> closeWindow;

  glm::mat4 transformMatrix{1};
};

}// namespace pf
#endif//REALISTIC_VOXEL_RENDERING_SRC_RENDERING_SIMPLESVORENDERER_H