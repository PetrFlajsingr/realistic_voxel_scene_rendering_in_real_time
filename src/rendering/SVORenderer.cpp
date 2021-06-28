//
// Created by petr on 1/5/21.
//

#include "SVORenderer.h"
#include "light_field_probes/GridProbeGenerator.h"
#include "logging/loggers.h"
#include <experimental/array>
#include <fmt/chrono.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <pf_common/ByteLiterals.h>
#include <pf_common/Visitor.h>
#include <pf_common/enums.h>
#include <pf_common/files.h>
#include <pf_glfw_vulkan/ui/GlfwWindow.h>
#include <pf_imgui/backends/ImGuiGlfwVulkanInterface.h>
#include <pf_imgui/elements/DockSpace.h>
#include <utils/FlameGraphSampler.h>
#include <voxel/SVO_utils.h>
#include <voxel/SceneFileManager.h>
#include <voxel/SparseVoxelOctreeCreation.h>
#include <voxel/TeardownMaps.h>

namespace pf {
using namespace vulkan;
using namespace pf::byte_literals;
using namespace ui::ig;

// TODO: fix memo race issues
std::ostream &operator<<(std::ostream &o, pf::Enum auto e) {
  o << magic_enum::enum_name(e);
  return o;
}

/**
 * Model info buffer:
 * uint model count
 * model infos
 *      vec3 scale
 *      uint SVO offset - scale and offset saved in vec4
 *      mat4 object matrix
 *      mat4 inverse object matrix
 */

// TODO: teardown map file loading
// TODO: conversion app for binary svos
// TODO: read raw svos from disk
SVORenderer::SVORenderer(toml::table &tomlConfig)
    : config(tomlConfig), camera({0, 0}, 0.001f, 2000.f, 2.5, 2.5, {1.4, 0.8, 2.24}, {0, 0, -1}, {0, -1, 0}) {
  computeLocalSize = std::pair{config.get()["rendering"]["compute"]["local_size_x"].value_or<std::size_t>(8),
                               config.get()["rendering"]["compute"]["local_size_y"].value_or<std::size_t>(8)};
}

SVORenderer::~SVORenderer() {
  if (vkLogicalDevice == nullptr) { return; }
  stop();
  window->setExceptionHandler([](auto) { return false; });
  log(spdlog::level::info, APP_TAG, "Destroying renderer, waiting for device");
  vkLogicalDevice->wait();
  log(spdlog::level::info, APP_TAG, "Saving UI to config");
  ui->imgui->updateConfig();
  config.get()["ui"].as_table()->insert_or_assign("imgui", ui->imgui->getConfig());
}
void SVORenderer::init(const std::shared_ptr<ui::Window> &win) {
  window = win;
  closeWindow = [this] { window->close(); };
  camera.setSwapLeftRight(false);
  pf::vulkan::setGlobalLoggerInstance(std::make_shared<GlobalLoggerInterface>("global_vulkan"));
  log(spdlog::level::info, APP_TAG, "Initialising Vulkan.");

  createInstance();
  createSurface();
  createDevices();

  buildVulkanObjects();

  GLFWwindow *windowHandle = nullptr;// FIXME
  if (auto glfwWindow = dynamic_cast<ui::GlfwWindow *>(window.get()); glfwWindow != nullptr) {
    windowHandle = glfwWindow->getHandle();
  }
  assert(windowHandle != nullptr);

  auto imguiConfig =
      config.get()["ui"].as_table()->contains("imgui") ? *config.get()["ui"]["imgui"].as_table() : toml::table{};
  auto imgui = std::make_unique<ui::ig::ImGuiGlfwVulkanInterface>(ui::ig::ImGuiVulkanGlfwConfig{
      .instance = **vkInstance,
      .physicalDevice = **vkDevice,
      .device = **vkLogicalDevice,
      .renderPass = **vkRenderPass,
      .surface = **vkSurface,
      .swapchain = **vkSwapChain,
      .graphicsQueue = vkLogicalDevice->getQueue(vk::QueueFlagBits::eGraphics),
      .presentQueue = vkLogicalDevice->getPresentQueue(),
      .swapchainImageCount = static_cast<std::uint32_t>(vkSwapChain->getImageViews().size()),
      .handle = windowHandle,
      .flags = {},
      .enableMultiViewport = false,
      .config = imguiConfig,
      .pathToIconFolder = *imguiConfig["path_icons"].value<std::string>(),
      .enabledIconPacks = IconPack::FontAwesome5Regular,
      .defaultFontSize = 13.f});

  window->addKeyboardListener(events::KeyEventType::Pressed, [this](const events::KeyEvent &event) {
    if (event.key == 'H') {
      switch (ui->imgui->getVisibility()) {
        case ui::ig::Visibility::Visible: ui->imgui->setVisibility(ui::ig::Visibility::Invisible); break;
        case ui::ig::Visibility::Invisible: ui->imgui->setVisibility(ui::ig::Visibility::Visible); break;
      }
      return true;
    }
    return false;
  });

  window->addTextListener([](events::TextEvent event) {
    logd(MAIN_TAG, event.text);
    return true;
  });

  camera.setScreenWidth(window->getResolution().width);
  camera.setScreenHeight(window->getResolution().height);
  window->setInputIgnorePredicate([this] { return ui->imgui->isWindowHovered() || ui->imgui->isKeyboardCaptured(); });
  camera.registerControls(*window);

  ui = std::make_unique<SimpleSVORenderer_UI>(
      std::move(imgui), window, camera, TextureData{*vkIterImage, *vkIterImageView, *vkIterImageSampler},
      TextureData{*probeRenderer->getProbesDebugImage(), *probeRenderer->getProbesDebugImageView(),
                  *probeRenderer->getProbesDebugSampler()});

  modelManager = std::make_unique<vox::GPUModelManager>(svoMemoryPool, modelInfoMemoryPool, materialMemoryPool, 5);

  auto probeMapping = probePosBuffer->mapping();
  const auto totalProbeCount = probeRenderer->probeManager->getTotalProbeCount();
  auto probePositions = probeRenderer->probeManager->getProbePositions();
  for (const auto &[idx, position] : probePositions | ranges::views::enumerate) {
    probeMapping.set(glm::vec4{position, totalProbeCount}, idx);
  }

  initUI();
  window->setMainLoopCallback([&] { render(); });
}

std::unordered_set<std::string> SVORenderer::getValidationLayers() {
  return std::unordered_set<std::string>{"VK_LAYER_KHRONOS_validation"};
}

void SVORenderer::buildVulkanObjects() {
  createBuffers();
  probeRenderer = std::make_unique<lfp::ProbeRenderer>(
      config.get(), vkInstance, vkDevice, vkLogicalDevice, svoBuffer, modelInfoBuffer, bvhBuffer, cameraUniformBuffer,
      materialBuffer,
      std::make_unique<lfp::ProbeManager>(glm::ivec3{4, 4, 4}, glm::vec3{-2, -2, -2}, 1.4f, glm::ivec3{128, 128, 128},
                                          vkLogicalDevice));

  createSwapchain();
  createTextures();
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

void SVORenderer::createInstance() {
  using namespace vulkan;
  using namespace vulkan::literals;
  const auto windowExtensions = window->requiredVulkanExtensions();
  auto validationLayers = getValidationLayers();
  vkInstance = Instance::CreateShared(
      InstanceConfig{.appName = "Realistic voxel rendering in real time",
                     .appVersion = "0.1.0"_v,
                     .vkVersion = "1.2.0"_v,
                     .engineInfo = EngineInfo{.name = "<unnamed>", .engineVersion = "0.1.0"_v},
                     .requiredWindowExtensions = windowExtensions,
                     .validationLayers = validationLayers,
                     .callback = [](const DebugCallbackData &data, vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                    const vk::DebugUtilsMessageTypeFlagsEXT &type_flags) {
                       return debugCallback(data, severity, type_flags);
                     }});
}

void SVORenderer::createSurface() { vkSurface = vkInstance->createSurface(window); }

void SVORenderer::render() {
  auto sampler = FlameGraphSampler{};
  auto mainSample = sampler.blockSampler("render loop");

  auto swapSample = mainSample.blockSampler("swap");
  vkSwapChain->swap();

  auto &semaphore = vkSwapChain->getCurrentSemaphore();
  auto &fence = vkSwapChain->getCurrentFence();

  fence.reset();
  vkComputeFence->reset();
  swapSample.end();

  auto imguiSample = mainSample.blockSampler("imgui");

  ui->imgui->render();
  imguiSample.end();

  auto commandRecordSample = mainSample.blockSampler("commandRecord");
  recordCommands();
  commandRecordSample.end();
  {
    auto cameraMapping = cameraUniformBuffer->mapping();
    cameraMapping.set(
        std::vector{glm::vec4{camera.getPosition(), 0}, glm::vec4{camera.getFront(), 0}, glm::vec4{camera.getUp(), 0}});
    cameraMapping.setRawOffset(camera.getViewMatrix(), sizeof(glm::vec4) * 3);
    cameraMapping.setRawOffset(camera.getProjectionMatrix(), sizeof(glm::vec4) * 3 + sizeof(glm::mat4));
    const auto invProjView = glm::inverse(camera.getViewMatrix()) * glm::inverse(camera.getProjectionMatrix());
    cameraMapping.setRawOffset(invProjView, sizeof(glm::vec4) * 3 + sizeof(glm::mat4) * 2);
    cameraMapping.setRawOffset(camera.getNear(), sizeof(glm::vec4) * 3 + sizeof(glm::mat4) * 3);
    cameraMapping.setRawOffset(camera.getFar(), sizeof(glm::vec4) * 3 + sizeof(glm::mat4) * 3 + sizeof(float));
  }

  const auto commandBufferIndex = vkSwapChain->getCurrentImageIndex();
  const auto frameIndex = vkSwapChain->getCurrentFrameIndex();

  auto probeSample = mainSample.blockSampler("probes");
  auto probeSemaphore = probeRenderer->render();
  probeSample.end();

  auto computeSample = mainSample.blockSampler("compute");
  vkCommandBuffers[commandBufferIndex]->submit(
      {.waitSemaphores = {semaphore, *probeSemaphore},
       .signalSemaphores = {*computeSemaphore},
       .flags = {vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eComputeShader},
       .fence = fence,
       .wait = true});

  fence.reset();
  computeSample.end();

  auto presentSample = mainSample.blockSampler("present");
  vkGraphicsCommandBuffers[commandBufferIndex]->submit({.waitSemaphores = {*computeSemaphore},
                                                        .signalSemaphores = {*renderSemaphores[frameIndex]},
                                                        .flags = {vk::PipelineStageFlagBits::eColorAttachmentOutput},
                                                        .fence = fence,
                                                        .wait = true});

  vkSwapChain->present(
      {.waitSemaphores = {*renderSemaphores[frameIndex]}, .presentQueue = vkLogicalDevice->getPresentQueue()});
  presentSample.end();
  vkSwapChain->frameDone();
  fpsCounter.onFrame();
  mainSample.end();
  ui->flameGraph.setSamples(sampler.getSamples());
}

void SVORenderer::createDevices() {
  vkDevice = vkInstance->selectDevice(DefaultDeviceSuitabilityScorer({}, {}, [](const vk::PhysicalDeviceFeatures &) {
    // FIXME: actually select the best one
    return 0;
  }));
  vkLogicalDevice =
      vkDevice->createLogicalDevice({.id = "dev1",
                                     .deviceFeatures = vk::PhysicalDeviceFeatures{},
                                     .queueTypes = {vk::QueueFlagBits::eCompute, vk::QueueFlagBits::eGraphics},
                                     .presentQueueEnabled = true,
                                     .requiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                                  VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME,
                                                                  VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME},
                                     .validationLayers = getValidationLayers(),
                                     .surface = *vkSurface});
}

void SVORenderer::createSwapchain() {
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
       .resolution = {window->getResolution().width, window->getResolution().height},
       .imageUsage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst
           | vk::ImageUsageFlagBits::eColorAttachment,
       .sharingQueues = {},
       .imageArrayLayers = 1,
       .clipped = true,
       .oldSwapChain = std::nullopt,
       .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque});
}

void SVORenderer::createTextures() {
  vkRenderImage = vkLogicalDevice->createImage(
      {.imageType = vk::ImageType::e2D,
       .format = vkSwapChain->getFormat(),
       .extent =
           vk::Extent3D{.width = vkSwapChain->getExtent().width, .height = vkSwapChain->getExtent().height, .depth = 1},
       .mipLevels = 1,
       .arrayLayers = 1,
       .sampleCount = vk::SampleCountFlagBits::e1,
       .tiling = vk::ImageTiling::eOptimal,
       .usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc
           | vk::ImageUsageFlagBits::eTransferDst,
       .sharingQueues = {},
       .layout = vk::ImageLayout::eUndefined});
  vkRenderImageView =
      vkRenderImage->createImageView(vk::ColorSpaceKHR::eSrgbNonlinear, vk::ImageViewType::e2D,
                                     vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
  vkIterImage = vkLogicalDevice->createImage(
      {.imageType = vk::ImageType::e2D,
       .format = vkSwapChain->getFormat(),
       .extent =
           vk::Extent3D{.width = vkSwapChain->getExtent().width, .height = vkSwapChain->getExtent().height, .depth = 1},
       .mipLevels = 1,
       .arrayLayers = 1,
       .sampleCount = vk::SampleCountFlagBits::e1,
       .tiling = vk::ImageTiling::eOptimal,
       .usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc
           | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
       .sharingQueues = {},
       .layout = vk::ImageLayout::eUndefined});
  vkIterImageView =
      vkIterImage->createImageView(vk::ColorSpaceKHR::eSrgbNonlinear, vk::ImageViewType::e2D,
                                   vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

  vkIterImageSampler = TextureSampler::CreateShared(
      vkLogicalDevice,
      TextureSamplerConfig{.magFilter = vk::Filter::eLinear,
                           .minFilter = vk::Filter::eLinear,
                           .addressMode = {.u = vk::SamplerAddressMode::eClampToBorder,
                                           .v = vk::SamplerAddressMode::eClampToBorder,
                                           .w = vk::SamplerAddressMode::eClampToBorder},
                           .maxAnisotropy = std::nullopt,
                           .borderColor = vk::BorderColor::eFloatOpaqueBlack,
                           .unnormalizedCoordinates = false,
                           .compareOp = std::nullopt,
                           .mip = {.mode = vk::SamplerMipmapMode::eLinear, .lodBias = 0, .minLod = 0, .maxLod = 1}});
}

void SVORenderer::createDescriptorPool() {
  vkDescPool = vkLogicalDevice->createDescriptorPool({.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                                      .maxSets = 1,
                                                      .poolSizes = {
                                                          {vk::DescriptorType::eStorageImage, 1}, // color
                                                          {vk::DescriptorType::eUniformBuffer, 1},// camera
                                                          {vk::DescriptorType::eUniformBuffer, 1},// light pos
                                                          {vk::DescriptorType::eStorageBuffer, 1},// svo
                                                          {vk::DescriptorType::eUniformBuffer, 1},// view type
                                                          {vk::DescriptorType::eStorageImage, 1}, // iterations
                                                          {vk::DescriptorType::eStorageBuffer, 1},// model infos
                                                          {vk::DescriptorType::eStorageBuffer, 1},// bvh
                                                          {vk::DescriptorType::eStorageBuffer, 1},// probes
                                                          {vk::DescriptorType::eStorageBuffer, 1},// materials
                                                      }});
}

void SVORenderer::createPipeline() {
  // TODO: compute pipeline builder
  // TODO: descriptor sets
  vkComputeDescSetLayout = vkLogicalDevice->createDescriptorSetLayout(
      {.bindings = {
           {.binding = 0,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// color
           {.binding = 1,
            .type = vk::DescriptorType::eUniformBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},//camera
           {.binding = 2,
            .type = vk::DescriptorType::eUniformBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// light pos
           {.binding = 3,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// svo
           {.binding = 4,
            .type = vk::DescriptorType::eUniformBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// debug
           {.binding = 5,
            .type = vk::DescriptorType::eStorageImage,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// iter
           {.binding = 6,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// model infos
           {.binding = 7,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// bvh
           {.binding = 8,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// probe positions
           {.binding = 9,
            .type = vk::DescriptorType::eStorageBuffer,
            .count = 1,
            .stageFlags = vk::ShaderStageFlagBits::eCompute},// materials
       }});

  const auto setLayouts = std::vector{**vkComputeDescSetLayout};
  const auto pipelineLayoutInfo =
      vk::PipelineLayoutCreateInfo{.setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
                                   .pSetLayouts = setLayouts.data()};
  auto computePipelineLayout = (*vkLogicalDevice)->createPipelineLayoutUnique(pipelineLayoutInfo);
  auto allocInfo = vk::DescriptorSetAllocateInfo{};
  allocInfo.setSetLayouts(setLayouts);
  allocInfo.descriptorPool = **vkDescPool;
  computeDescriptorSets = (*vkLogicalDevice)->allocateDescriptorSetsUnique(allocInfo);

  const auto computeColorInfo = vk::DescriptorImageInfo{.sampler = {},
                                                        .imageView = **vkRenderImageView,
                                                        .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeColorWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                        .dstBinding = 0,
                                                        .dstArrayElement = {},
                                                        .descriptorCount = 1,
                                                        .descriptorType = vk::DescriptorType::eStorageImage,
                                                        .pImageInfo = &computeColorInfo};

  const auto uniformCameraInfo =
      vk::DescriptorBufferInfo{.buffer = **cameraUniformBuffer, .offset = 0, .range = cameraUniformBuffer->getSize()};
  const auto uniformCameraWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                         .dstBinding = 1,
                                                         .dstArrayElement = {},
                                                         .descriptorCount = 1,
                                                         .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                         .pBufferInfo = &uniformCameraInfo};

  const auto lightPosInfo =
      vk::DescriptorBufferInfo{.buffer = **lightUniformBuffer, .offset = 0, .range = lightUniformBuffer->getSize()};
  const auto lightPosWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                    .dstBinding = 2,
                                                    .dstArrayElement = {},
                                                    .descriptorCount = 1,
                                                    .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                    .pBufferInfo = &lightPosInfo};

  const auto svoInfo = vk::DescriptorBufferInfo{.buffer = **svoBuffer, .offset = 0, .range = svoBuffer->getSize()};
  const auto svoWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                               .dstBinding = 3,
                                               .dstArrayElement = {},
                                               .descriptorCount = 1,
                                               .descriptorType = vk::DescriptorType::eStorageBuffer,
                                               .pBufferInfo = &svoInfo};

  const auto uniformDebugInfo =
      vk::DescriptorBufferInfo{.buffer = **debugUniformBuffer, .offset = 0, .range = debugUniformBuffer->getSize()};
  const auto uniformDebugWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                        .dstBinding = 4,
                                                        .dstArrayElement = {},
                                                        .descriptorCount = 1,
                                                        .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                        .pBufferInfo = &uniformDebugInfo};

  const auto computeIterInfo =
      vk::DescriptorImageInfo{.sampler = {}, .imageView = **vkIterImageView, .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeIterWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                       .dstBinding = 5,
                                                       .dstArrayElement = {},
                                                       .descriptorCount = 1,
                                                       .descriptorType = vk::DescriptorType::eStorageImage,
                                                       .pImageInfo = &computeIterInfo};

  const auto modelInfoInfo =
      vk::DescriptorBufferInfo{.buffer = **modelInfoBuffer, .offset = 0, .range = modelInfoBuffer->getSize()};
  const auto modelInfoWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                     .dstBinding = 6,
                                                     .dstArrayElement = {},
                                                     .descriptorCount = 1,
                                                     .descriptorType = vk::DescriptorType::eStorageBuffer,
                                                     .pBufferInfo = &modelInfoInfo};

  const auto bvhInfo = vk::DescriptorBufferInfo{.buffer = **bvhBuffer, .offset = 0, .range = bvhBuffer->getSize()};
  const auto bvhWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                               .dstBinding = 7,
                                               .dstArrayElement = {},
                                               .descriptorCount = 1,
                                               .descriptorType = vk::DescriptorType::eStorageBuffer,
                                               .pBufferInfo = &bvhInfo};

  const auto probePosInfo =
      vk::DescriptorBufferInfo{.buffer = **probePosBuffer, .offset = 0, .range = probePosBuffer->getSize()};
  const auto probePosWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                    .dstBinding = 8,
                                                    .dstArrayElement = {},
                                                    .descriptorCount = 1,
                                                    .descriptorType = vk::DescriptorType::eStorageBuffer,
                                                    .pBufferInfo = &probePosInfo};

  const auto materialsInfo =
      vk::DescriptorBufferInfo{.buffer = **materialBuffer, .offset = 0, .range = materialBuffer->getSize()};
  const auto materialsWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                     .dstBinding = 9,
                                                     .dstArrayElement = {},
                                                     .descriptorCount = 1,
                                                     .descriptorType = vk::DescriptorType::eStorageBuffer,
                                                     .pBufferInfo = &materialsInfo};

  const auto writeSets =
      std::vector{computeColorWrite, uniformCameraWrite, lightPosWrite, svoWrite,      uniformDebugWrite,
                  computeIterWrite,  modelInfoWrite,     bvhWrite,      probePosWrite, materialsWrite};
  (*vkLogicalDevice)->updateDescriptorSets(writeSets, nullptr);

  auto computeShader = vkLogicalDevice->createShader(ShaderConfigGlslFile{
      .name = "SVO",
      .type = ShaderType::Compute,
      .path = (std::filesystem::path(*config.get()["resources"]["path_shaders"].value<std::string>()) /= "svo_bvh.comp")
                  .string(),
      .macros = {},
      .replaceMacros = {{"LOCAL_SIZE_X", std::to_string(computeLocalSize.first)},
                        {"LOCAL_SIZE_Y", std::to_string(computeLocalSize.second)}}});

  const auto computeStageInfo = vk::PipelineShaderStageCreateInfo{.stage = computeShader->getVkType(),
                                                                  .module = **computeShader,
                                                                  .pName = "main"};
  const auto pipelineInfo = vk::ComputePipelineCreateInfo{.stage = computeStageInfo, .layout = *computePipelineLayout};
  vkComputePipeline = ComputePipeline::CreateShared(
      (*vkLogicalDevice)->createComputePipelineUnique(nullptr, pipelineInfo).value, std::move(computePipelineLayout));
}
void SVORenderer::createCommands() {
  vkCommandPool = vkLogicalDevice->createCommandPool(
      {.queueFamily = vk::QueueFlagBits::eCompute, .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer});

  vkRenderImage->transitionLayout(*vkCommandPool, vk::ImageLayout::eGeneral,
                                  vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
  vkIterImage->transitionLayout(*vkCommandPool, vk::ImageLayout::eGeneral,
                                vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

  vkCommandBuffers = vkCommandPool->createCommandBuffers({.level = vk::CommandBufferLevel::ePrimary, .count = 3});

  vkGraphicsCommandPool = vkLogicalDevice->createCommandPool(
      {.queueFamily = vk::QueueFlagBits::eGraphics, .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer});

  vkGraphicsCommandBuffers = vkGraphicsCommandPool->createCommandBuffers(
      {.level = vk::CommandBufferLevel::ePrimary,
       .count = static_cast<uint32_t>(vkSwapChain->getFrameBuffers().size())});
}
void SVORenderer::recordCommands() {
  // TODO: add these calls to recording
  static bool isComputeRecorded = false;

  if (!isComputeRecorded) {
    isComputeRecorded = true;
    for (auto i : std::views::iota(0ul, vkCommandBuffers.size())) {
      auto &buffer = vkCommandBuffers[i];
      auto recording = buffer->begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
      recording.bindPipeline(vk::PipelineBindPoint::eCompute, *vkComputePipeline);

      const auto vkDescSets = computeDescriptorSets
          | ranges::views::transform([](const auto &descSet) { return *descSet; }) | ranges::to_vector;
      recording.getCommandBuffer()->bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                                       vkComputePipeline->getVkPipelineLayout(), 0, vkDescSets, {});
      recording.dispatch(vkSwapChain->getExtent().width / computeLocalSize.first,
                         vkSwapChain->getExtent().height / computeLocalSize.second, 1);
      auto &currentSwapchainImage = *vkSwapChain->getImages()[i];
      {
        auto imageBarriers = std::vector<vk::ImageMemoryBarrier>{};
        imageBarriers.emplace_back(vkRenderImage->createImageBarrier(
            {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}, {}, vk::AccessFlagBits::eShaderWrite,
            vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral));
        imageBarriers.emplace_back(vkRenderImage->createImageBarrier(
            {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}, vk::AccessFlagBits::eShaderWrite,
            vk::AccessFlagBits::eTransferRead, vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal));
        imageBarriers.emplace_back(currentSwapchainImage.createImageBarrier(
            {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}, {}, vk::AccessFlagBits::eTransferWrite,
            vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal));
        recording.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eAllCommands,
                                  {}, {}, imageBarriers);
      }

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
            vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eMemoryRead, vk::ImageLayout::eTransferDstOptimal,
            vk::ImageLayout::ePresentSrcKHR));
        imageBarriers.emplace_back(vkRenderImage->createImageBarrier(
            {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}, vk::AccessFlagBits::eTransferRead, {},
            vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eGeneral));
        recording.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eTopOfPipe, {},
                                  {}, imageBarriers);
      }
    }
  }

  auto graphRecording = vkGraphicsCommandBuffers[vkSwapChain->getCurrentImageIndex()]->begin(
      vk::CommandBufferUsageFlagBits::eRenderPassContinue);

  graphRecording.beginRenderPass({.renderPass = *vkRenderPass,
                                  .frameBuffer = *vkSwapChain->getFrameBuffers()[vkSwapChain->getCurrentImageIndex()],
                                  .clearValues = {},
                                  .extent = vkSwapChain->getExtent()});
  ui->imgui->addToCommandBuffer(*graphRecording.getCommandBuffer());
  graphRecording.endRenderPass();
}
void SVORenderer::createFences() {
  vkComputeFence = vkLogicalDevice->createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
  std::ranges::generate_n(std::back_inserter(fences), vkSwapChain->getFrameBuffers().size(),
                          [&] { return vkLogicalDevice->createFence({.flags = vk::FenceCreateFlagBits::eSignaled}); });
}
void SVORenderer::createSemaphores() {
  computeSemaphore = vkLogicalDevice->createSemaphore();
  std::ranges::generate_n(std::back_inserter(renderSemaphores), vkSwapChain->getFrameBuffers().size(),
                          [&] { return vkLogicalDevice->createSemaphore(); });
}
void SVORenderer::initUI() {
  using namespace std::string_literals;
  using namespace pf::ui::ig;
  using namespace pf::enum_operators;

  window->setExceptionHandler([this](const std::exception &e) {
    ui->imgui->createMsgDlg("Exception thrown", fmt::format("An exception has been thrown: \n{}", e.what()),
                            Flags{MessageButtons::Ok}, [](auto) { return true; });
    return true;
  });

  //FIXME
  ui->openModelMenuItem.addClickListener([this] {
    ui->imgui->openFileDialog(
        "Select model", {FileExtensionSettings{{"vox", "pf_vox"}, "Vox model", ImVec4{1, 0, 0, 1}}},
        [this](const auto &selected) {
          auto modelPath = selected[0];
          const auto &[loadingDialog, loadingProgress, loadingText] = ui->createLoadingDialog();
          threadpool->template enqueue([this, modelPath, &loadingProgress, &loadingDialog, &loadingText] {
            auto newModel = modelManager->loadModel(
                modelPath, {[this, &loadingProgress](float progress) {
                  window->enqueue([&loadingProgress, progress] { loadingProgress.setValue(progress); });
                }},
                !ui->modelLoadingSeparateModelsCheckbox.getValue(), true);
            if (!newModel.has_value()) {
              const auto message = newModel.error();
              window->enqueue([&loadingDialog, &loadingText, message] {
                loadingText.setText("Loading failed: {}", message);
                loadingDialog.createChild<Button>(uniqueId(), "Ok").addClickListener([&loadingDialog] {
                  loadingDialog.close();
                });
              });
            } else {
              auto modelPtrs = newModel.value();
              window->enqueue([this, &loadingDialog, modelPtrs] {
                for (auto modelPtr : modelPtrs) {
                  auto newUIItem = ModelFileInfo{modelPtr->path};
                  newUIItem.modelData = modelPtr;
                  auto &itemSelectable = ui->activeModelList.addItem(newUIItem);
                  addActiveModelPopupMenu(itemSelectable, newUIItem.id, modelPtr);
                  modelPtr->updateInfoToGPU();
                }
                loadingDialog.close();
                rebuildAndUploadBVH();
              });
            }
          });
        },
        [] {}, Size{500, 400}, std::filesystem::path(*config.get()["resources"]["path_models"].value<std::string>()));
  });

  ui->closeMenuItem.addClickListener(closeWindow);

  ui->viewTypeComboBox.addValueListener(
      [this](const auto &viewType) {
        const auto viewTypeIdx = static_cast<int>(viewType);
        debugUniformBuffer->mapping().set(viewTypeIdx);
      },
      true);

  ui->lightPosSlider.addValueListener(
      [&](auto pos) {
        pos.y *= -1;
        lightUniformBuffer->mapping().set(pos);
      },
      true);

  ui->shadowsCheckbox.addValueListener([this](auto enabled) { debugUniformBuffer->mapping().set(enabled ? 1 : 0, 2); },
                                       true);

  ui->shaderDebugFloatValueSlider.addValueListener([this](auto value) { debugUniformBuffer->mapping().set(value, 3); },
                                                   true);

  ui->modelDetailTranslateDrag.addValueListener([this](const auto &val) {
    if (auto selectedItem = ui->activeModelList.getSelectedItem(); selectedItem.has_value()) {
      selectedItem->get().modelData->translateVec = val;
      selectedItem->get().modelData->updateInfoToGPU();
      rebuildAndUploadBVH();
    }
  });
  ui->modelDetailRotateDrag.addValueListener([this](const auto &val) {
    if (auto selectedItem = ui->activeModelList.getSelectedItem(); selectedItem.has_value()) {
      selectedItem->get().modelData->rotateVec = val;
      selectedItem->get().modelData->updateInfoToGPU();
      rebuildAndUploadBVH();
    }
  });
  ui->modelDetailScaleDrag.addValueListener([this](const auto &val) {
    if (auto selectedItem = ui->activeModelList.getSelectedItem(); selectedItem.has_value()) {
      selectedItem->get().modelData->scaleVec = val;
      selectedItem->get().modelData->updateInfoToGPU();
      rebuildAndUploadBVH();
    }
  });

  ui->shaderDebugIterDivideDrag.addValueListener(
      [this](auto value) { debugUniformBuffer->mapping().setRawOffset(value, sizeof(int) * 3 + sizeof(float)); }, true);

  ui->ambientColPicker.addValueListener(
      [&](const auto &ambientColor) {
        lightUniformBuffer->mapping().set(glm::vec4{ambientColor, 1}, 1);
      },
      true);

  ui->diffuseColPicker.addValueListener(
      [&](const auto &diffuseColor) {
        lightUniformBuffer->mapping().set(glm::vec4{diffuseColor, 1}, 2);
      },
      true);

  ui->specularColPicker.addValueListener(
      [&](const auto &specularColor) {
        lightUniformBuffer->mapping().set(glm::vec4{specularColor, 1}, 3);
      },
      true);

  const auto modelsPath = std::filesystem::path(*config.get()["resources"]["path_models"].value<std::string>());
  const auto modelFileNames = loadModelFileNames(modelsPath);
  ui->modelList.setItems(modelFileNames | std::views::transform([](const auto &path) { return ModelFileInfo{path}; }));

  ui->activeModelList.addValueListener([this](const auto &modelInfo) {
    const uint32_t index = modelInfo.modelData->getModelIndex().value();
    debugUniformBuffer->mapping().template set(index, 1);
  });

  debugUniformBuffer->mapping().template set(-1, 1);
  ui->activeModelList.addDropListener([this](const auto &modelInfo) {
    const auto &[loadingDialog, loadingProgress, loadingText] = ui->createLoadingDialog();
    threadpool->template enqueue([this, modelInfo, &loadingProgress, &loadingDialog, &loadingText] {
      auto newModel = modelManager->loadModel(
          modelInfo.path, {[this, &loadingProgress](float progress) {
            window->enqueue([&loadingProgress, progress] { loadingProgress.setValue(progress); });
          }},
          !ui->modelLoadingSeparateModelsCheckbox.getValue(), true);
      if (!newModel.has_value()) {
        const auto message = newModel.error();
        window->enqueue([this, modelInfo, &loadingDialog, &loadingText, message] {
          loadingText.setText("Loading failed: {}", message);
          loadingDialog.createChild<Button>(uniqueId(), "Ok").addClickListener([&loadingDialog] {
            loadingDialog.close();
          });
          ui->activeModelList.removeItem(modelInfo);
        });
      } else {
        auto modelPtrs = newModel.value();
        window->enqueue([this, &loadingDialog, modelInfo, modelPtrs] {
          for (auto modelPtr : modelPtrs) {
            ui->activeModelList.removeItem(modelInfo);
            auto newUIItem = ModelFileInfo{modelPtr->path};
            newUIItem.modelData = modelPtr;
            const auto itemId = newUIItem.id;
            auto &itemSelectable = ui->activeModelList.addItem(newUIItem);
            addActiveModelPopupMenu(itemSelectable, itemId, modelPtr);
            modelPtr->updateInfoToGPU();
          }
          loadingDialog.close();
          rebuildAndUploadBVH();
        });
      }
    });
  });

  ui->activateSelectedModelButton.addClickListener([this] {
    if (auto item = ui->modelList.getSelectedItem(); item.has_value()) {
      const auto &[loadingDialog, loadingProgress, loadingText] = ui->createLoadingDialog();
      threadpool->template enqueue([this, item, &loadingProgress, &loadingDialog, &loadingText] {
        auto newModel = modelManager->loadModel(
            item->get().path, {[this, &loadingProgress](float progress) {
              window->enqueue([&loadingProgress, progress] { loadingProgress.setValue(progress); });
            }},
            !ui->modelLoadingSeparateModelsCheckbox.getValue(), true);
        if (!newModel.has_value()) {
          const auto message = newModel.error();
          window->enqueue([&loadingDialog, &loadingText, message] {
            loadingText.setText("Loading failed: {}", message);
            loadingDialog.createChild<Button>(uniqueId(), "Ok").addClickListener([&loadingDialog] {
              loadingDialog.close();
            });
          });
        } else {
          auto modelPtrs = newModel.value();
          window->enqueue([this, &loadingDialog, modelPtrs] {
            for (auto modelPtr : modelPtrs) {
              auto newUIItem = ModelFileInfo{modelPtr->path};
              newUIItem.modelData = modelPtr;
              auto &itemSelectable = ui->activeModelList.addItem(newUIItem);
              addActiveModelPopupMenu(itemSelectable, newUIItem.id, modelPtr);
              modelPtr->updateInfoToGPU();
            }
            loadingDialog.close();
            rebuildAndUploadBVH();
          });
        }
      });
    }
  });

  ui->reloadModelListButton.addClickListener([modelsPath, this] {
    const auto modelFileNames = loadModelFileNames(modelsPath);
    ui->modelList.setItems(modelFileNames
                           | std::views::transform([](const auto &path) { return ModelFileInfo{path}; }));
  });

  ui->shaderDebugValueInput.addValueListener([this](const auto &val) { debugUniformBuffer->mapping().set(val, 1); },
                                             true);

  // FIXME
  //subscriptions.emplace_back(addLogListener([this](auto record) { ui->logMemo.addRecord(record); }));
  //subscriptions.emplace_back(addLogListener([this](auto record) { ui->logErrMemo.addRecord(record); }, true));

  ui->chaiConfirmButton.addClickListener([this] {
    const auto input = ui->chaiInputText.getText();
    ui->chaiOutputMemo.addRecord(">>> "s + input);
    ui->chaiInputText.clear();
    try {
      chai->eval(input);
    } catch (const chaiscript::exception::eval_error &e) { ui->chaiOutputMemo.addRecord("<<< "s + e.pretty_print()); }
  });

  chai->add(chaiscript::fun([](const std::string &str) { log(spdlog::level::debug, APP_TAG, str); }), "log");

  const auto fpsMsgTemplate = "FPS:\nCurrent: {:0.2f}\nAverage: {:0.2f}";

  ui->resetFpsButton.addClickListener([this] {
    fpsCounter.reset();
    ui->fpsCurrentPlot.clear();
    ui->fpsAveragePlot.clear();
  });

  ui->vsyncCheckbox.addValueListener([](auto enabled) { logd("UI", "Vsync enabled: {}", enabled); }, true);

  const auto cameraPosTemplate = "Position: {0:0.2f}x{1:0.2f}x{2:0.2f}";
  const auto cameraDirTemplate = "Direction: {0:0.2f}x{1:0.2f}x{2:0.2f}";

  ui->cameraMoveSpeedSlider.addValueListener([this](auto value) { camera.setMovementSpeed(value); }, true);
  ui->cameraMouseSpeedSlider.addValueListener([this](auto value) { camera.setMouseSpeed(value); }, true);
  ui->cameraFOVSlider.addValueListener([this](auto value) { camera.setFieldOfView(value); }, true);

  fpsCounter.setOnNewFrame([this, cameraDirTemplate, cameraPosTemplate, fpsMsgTemplate](const FPSCounter &counter) {
    static auto cntSinceLast = 0;
    if (cntSinceLast > counter.currentFPS() / 10) {
      ui->fpsCurrentPlot.addValue(counter.currentFPS());
      ui->fpsAveragePlot.addValue(counter.averageFPS());
      cntSinceLast = 0;
    }
    ++cntSinceLast;
    ui->fpsLabel.setText(fmt::format(fpsMsgTemplate, counter.currentFPS(), counter.averageFPS()));
    const auto camPos = camera.getPosition();
    ui->cameraPosText.setText(fmt::format(cameraPosTemplate, camPos.x, camPos.y, camPos.z));
    const auto camDir = camera.getFront();
    ui->cameraDirText.setText(fmt::format(cameraDirTemplate, camDir.x, camDir.y, camDir.z));
  });

  ui->clearActiveModelsButton.addClickListener([this] {
    ui->imgui->createMsgDlg("Clear all models", "Do you really want to remove all models?",
                            MessageButtons::Yes | MessageButtons::No, [this](auto btn) {
                              if (btn == MessageButtons::Yes) {
                                std::ranges::for_each(ui->activeModelList.getItems(), [this](const auto &item) {
                                  modelManager->removeModel(item.modelData);
                                });
                                ui->activeModelList.setItems<std::vector<ModelFileInfo>>({});
                                rebuildAndUploadBVH();
                              }
                              return true;
                            });
  });

  ui->debugPrintEnableCheckbox.addValueListener(
      [this](auto enabled) { debugUniformBuffer->mapping(sizeof(std::uint32_t) * 5).set(enabled ? 1 : 0); }, true);

  ui->bvhVisualizeCheckbox.addValueListener(
      [this](auto enabled) { debugUniformBuffer->mapping(sizeof(std::uint32_t) * 6).set(enabled ? 1 : 0); }, true);
  ui->visualizeProbesCheckbox.addValueListener(
      [this](auto enabled) { debugUniformBuffer->mapping(sizeof(std::uint32_t) * 7).set(enabled ? 1 : 0); }, true);

  // FIXME
  ui->loadSceneMenuItem.addClickListener([this] {
    ui->imgui->openFileDialog(
        "Select file to load scene info", {FileExtensionSettings{{"toml"}, "toml", ImVec4{1, 0, 0, 1}}},
        [this](const auto &selected) {
          auto path = selected[0];
          auto models = vox::loadSceneFromFile(path);
          const auto &[loadingDialog, loadingProgress, loadingText] = ui->createLoadingDialog();
          threadpool->enqueue([this, models, &loadingDialog, &loadingProgress, &loadingText] {
            auto failed = false;
            auto loadedItems = std::vector<ModelFileInfo>{};
            auto currentModel = 0;
            const auto modelCount = static_cast<float>(models.size());
            std::ranges::for_each(
                models,
                [this, &currentModel, modelCount, &failed, &loadingDialog, &loadingProgress, &loadingText,
                 &loadedItems](const auto &modelInfo) {
                  ++currentModel;
                  if (auto iter = std::ranges::find_if(
                          loadedItems, [&modelInfo](const auto &loaded) { return modelInfo.path == loaded.path; });
                      iter != loadedItems.end()) {
                    auto originalModel = *iter;
                    auto duplicateResult = modelManager->createModelInstance(originalModel.modelData);
                    if (!duplicateResult.has_value()) {
                      failed = true;
                      const auto message = duplicateResult.error();
                      window->enqueue([this, message, modelInfo]() mutable {
                        loge(MAIN_TAG, "Error while duplicating SVO: {}", message);
                        ui->imgui->createMsgDlg("Duplication failed", fmt::format("Duplication failed: {}", message),
                                                Flags{ui::ig::MessageButtons::Ok}, [](auto) { return true; });
                      });
                    } else {
                      auto modelPtr = duplicateResult.value();
                      auto newUIItem = ModelFileInfo{modelPtr->path};
                      newUIItem.modelData = modelPtr;
                      modelPtr->translateVec = modelInfo.translateVec;
                      modelPtr->scaleVec = modelInfo.scaleVec;
                      modelPtr->rotateVec = modelInfo.rotateVec;
                      modelPtr->updateInfoToGPU();
                      const auto fileName = newUIItem.path.filename().string();
                      window->enqueue(
                          [this, currentModel, modelCount, fileName, &loadingProgress, newUIItem, modelPtr]() {
                            auto &itemSelectable = ui->activeModelList.addItem(newUIItem);
                            addActiveModelPopupMenu(itemSelectable, newUIItem.id, modelPtr);
                            loadingProgress.setValue(currentModel / modelCount * 100);
                          });
                    }
                  } else {
                    auto modelLoadResult = modelManager->loadModel(
                        modelInfo.path, {[this, &loadingProgress, currentModel, modelCount](float progress) {
                          window->enqueue([&loadingProgress, currentModel, modelCount, progress] {
                            loadingProgress.setValue(progress * (currentModel / modelCount));
                          });
                        }},
                        !ui->modelLoadingSeparateModelsCheckbox.getValue());
                    if (!modelLoadResult.has_value()) {
                      failed = true;
                      const auto message = modelLoadResult.error();
                      window->enqueue([message, &loadingText] {
                        loadingText.setText("{}\nLoading failed: {}", loadingText.getText(), message);
                      });
                    } else {
                      auto modelPtrs = modelLoadResult.value();
                      for (auto modelPtr : modelPtrs) {
                        auto newUIItem = ModelFileInfo{modelPtr->path};
                        newUIItem.modelData = modelPtr;
                        modelPtr->translateVec = modelInfo.translateVec;
                        modelPtr->scaleVec = modelInfo.scaleVec;
                        modelPtr->rotateVec = modelInfo.rotateVec;
                        modelPtr->updateInfoToGPU();
                        loadedItems.template emplace_back(newUIItem);
                        const auto fileName = newUIItem.path.filename().string();
                        window->enqueue([this, fileName, &loadingText, newUIItem, modelPtr]() {
                          auto &itemSelectable = ui->activeModelList.addItem(newUIItem);
                          addActiveModelPopupMenu(itemSelectable, newUIItem.id, modelPtr);
                          loadingText.setText("{}\nLoaded: {}", loadingText.getText(), fileName);
                        });
                      }
                    }
                  }
                });
            rebuildAndUploadBVH();
            if (failed) {
              window->enqueue([&loadingDialog] {
                loadingDialog.createChild<Button>(uniqueId(), "Ok").addClickListener([&loadingDialog] {
                  loadingDialog.close();
                });
              });
            } else {
              window->enqueue([&loadingDialog] { loadingDialog.close(); });
            }
          });
        },
        [] {});
  });

  ui->cameraToOriginButton.addClickListener([this] { camera.setPosition({0, 0, 0}); });

  ui->saveSceneMenuItem.addClickListener([this] {
    ui->imgui->openFileDialog(
        "Select file to save scene info", {FileExtensionSettings{{"toml"}, "toml", ImVec4{1, 0, 0, 1}}},
        [this](const auto &selected) {
          auto path = selected[0];
          vox::saveSceneToFile(modelManager->getModels(), path);
        },
        [] {});
  });

  ui->svoConverterMenuItem.addClickListener([this] {
    ui->createConvertWindow(*threadpool,
                            std::filesystem::path(*config.get()["resources"]["path_models"].value<std::string>()),
                            [this](const auto &src, const auto &dir) { convertAndSaveSVO(src, dir); });
  });

  ui->probeTextureCombobox.addValueListener([this](const auto type) { probeRenderer->setProbeDebugRenderType(type); },
                                            true);

  ui->teardownMapMenuItem.addClickListener([this] {
    ui->imgui->openDirDialog(
        "Select file to load scene info",
        [this]([[maybe_unused]] const auto &selected) {
          /*const auto sceneFolder = selected[0];
          auto doc = tinyxml2::XMLDocument{};
          doc.LoadFile((sceneFolder / "main.xml").string().c_str());
          [[maybe_unused]] auto scene = TeardownMap::Scene::FromXml(doc.RootElement(), sceneFolder);
          [[maybe_unused]] auto dataGroup = scene.toVoxDataGroup();
          std::unordered_map<std::string, std::unique_ptr<pf::vox::RawVoxelScene>> fileCache{};
          dataGroup.loadRawVoxelData(fileCache);

          std::function<void(glm::vec3, glm::vec3, TeardownMap::VoxDataGroup &)> c;
          c = [this, &c](glm::vec3 offset, glm::vec3 scale, TeardownMap::VoxDataGroup &group) {
            for (auto &chGroup : group.groups) { c(offset + group.position, scale * chGroup.scale, chGroup); }
            for (auto &data : group.voxData) {
              std::visit(Visitor{[&, this](std::unique_ptr<vox::RawVoxelModel> &model) {
                                   auto modelLoadResult = modelManager->loadModel(*model);
                                   auto modelPtr = modelLoadResult.value();
                                   auto newUIItem = ModelFileInfo{modelPtr->path};
                                   newUIItem.modelData = modelPtr;
                                   modelPtr->translateVec = (data.position + offset);
                                   modelPtr->scaleVec = scale * data.scale;
                                   modelPtr->rotateVec = glm::vec3{0};
                                   modelPtr->updateInfoToGPU();
                                   const auto fileName = newUIItem.path.filename().string();
                                   auto &itemSelectable = ui->activeModelList.addItem(newUIItem);
                                   addActiveModelPopupMenu(itemSelectable, newUIItem.id, modelPtr);
                                 },
                                 [&, this](std::unique_ptr<vox::RawVoxelScene> &scene) {
                                   auto modelLoadResult = modelManager->loadModel(*scene);
                                   auto modelPtrs = modelLoadResult.value();
                                   for (auto modelPtr : modelPtrs) {
                                     auto newUIItem = ModelFileInfo{modelPtr->path};
                                     newUIItem.modelData = modelPtr;
                                     modelPtr->translateVec = (data.position + offset);
                                     modelPtr->scaleVec = scale * data.scale;
                                     modelPtr->rotateVec = glm::vec3{0};
                                     modelPtr->updateInfoToGPU();
                                     const auto fileName = newUIItem.path.filename().string();
                                     auto &itemSelectable = ui->activeModelList.addItem(newUIItem);
                                     addActiveModelPopupMenu(itemSelectable, newUIItem.id, modelPtr);
                                   }
                                 }},
                         data.rawVoxelData);
            }
          };
          c(glm::vec3{0, 0, 0}, glm::vec3{1, 1, 1}, dataGroup);

          rebuildAndUploadBVH();*/
        },
        [] {}, Size{500, 400}, *config.get()["resources"]["path_models"].value<std::string>());
  });

  ui->renderProbesButton.addClickListener([this] { probeRenderer->renderProbesInNextPass(); });
  ui->selectedProbeSpinner.addValueListener([this](auto val) {
    probeRenderer->setProbeToRender(val);
    debugUniformBuffer->mapping().set(val, 8);
  });

  ui->probesDebugIntSpinner.addValueListener([this](auto val) { probeRenderer->setShaderDebugInt(val); });

  ui->fillProbeHolesButton.addValueListener([this](auto checked) { probeRenderer->setFillHoles(checked); });

  ui->imgui->setStateFromConfig();
}
std::vector<std::filesystem::path> SVORenderer::loadModelFileNames(const std::filesystem::path &dir) {
  const auto potentialModelFiles = filesInFolder(dir);
  return potentialModelFiles | ranges::views::filter([](const auto &path) {
           return isIn(path.extension(), std::vector{".vox", ".pf_vox"});
         })
      | ranges::to_vector | ranges::actions::sort;
}

void SVORenderer::stop() {
  for (auto &subscription : subscriptions) { subscription.unsubscribe(); }
  subscriptions.clear();
}

void SVORenderer::rebuildAndUploadBVH() {
  auto totalModels = std::size_t{};
  auto totalVoxels = std::size_t{};
  std::ranges::for_each(modelManager->getModels(), [&totalModels, &totalVoxels](const auto &model) {
    ++totalModels;
    totalVoxels += model.minimizedVoxelCount;
  });

  const auto &bvhTree = modelManager->rebuildBVH(true);

  const auto nodeCount = bvhTree.nodeCount;
  const auto depth = bvhTree.depth;
  ui->sceneVoxelCountText.setText(SimpleSVORenderer_UI::SCENE_VOXEL_COUNT_INFO, totalVoxels);
  ui->sceneModelCountText.setText(SimpleSVORenderer_UI::SCENE_MODEL_COUNT_INFO, totalModels);
  ui->sceneBVHNodeCountText.setText(SimpleSVORenderer_UI::SCENE_BVH_NODE_COUNT_INFO, nodeCount);
  ui->sceneBVHDepthText.setText(SimpleSVORenderer_UI::SCENE_BVH_DEPTH_INFO, depth);

  auto mapping = bvhBuffer->mapping();
  vox::saveBVHToBuffer(bvhTree.data, mapping);
}

std::function<void()> SVORenderer::popupClickActiveModel(std::size_t itemId, vox::GPUModelManager::ModelPtr modelPtr) {
  return [=, this] {
    window->enqueue([this, itemId, modelPtr] {
      const auto idToRemove = itemId;
      auto modelPtrToRemove = modelPtr;
      modelManager->removeModel(modelPtrToRemove);
      ui->activeModelList.removeItemIf([idToRemove](const auto &modelInfo) { return modelInfo.id == idToRemove; });
      rebuildAndUploadBVH();
    });
  };
}
void SVORenderer::addActiveModelPopupMenu(ui::ig::Selectable &element, std::size_t itemId,
                                          vox::GPUModelManager::ModelPtr modelPtr) {
  auto &itemPopupMenu = element.createPopupMenu();
  itemPopupMenu.addItem<MenuButtonItem>(uniqueId(), "Remove").addClickListener(popupClickActiveModel(itemId, modelPtr));
  itemPopupMenu.addItem<MenuButtonItem>(uniqueId(), "Duplicate").addClickListener([modelPtr, this] {
    duplicateModel(modelPtr);
  });
  itemPopupMenu.addItem<MenuButtonItem>(uniqueId(), "Create instance").addClickListener([modelPtr, this] {
    instantiateModel(modelPtr);
  });
}

void SVORenderer::duplicateModel(vox::GPUModelManager::ModelPtr original) {
  threadpool->enqueue([this, original] {
    auto newInstancePtr = modelManager->duplicateModel(original);
    if (!newInstancePtr.has_value()) {
      const auto message = newInstancePtr.error();
      window->enqueue([message, this] {
        ui->imgui->createMsgDlg("Error while duplicating a model", "Duplicate could not be created: {}",
                                Flags{MessageButtons::Ok}, [](auto) { return true; });
      });
    } else {
      auto modelPtr = newInstancePtr.value();
      window->enqueue([this, modelPtr] {
        auto newUIItem = ModelFileInfo{modelPtr->path};
        newUIItem.modelData = modelPtr;
        auto &itemSelectable = ui->activeModelList.addItem(newUIItem);
        addActiveModelPopupMenu(itemSelectable, newUIItem.id, modelPtr);
      });
    }
  });
}
void SVORenderer::instantiateModel(vox::GPUModelManager::ModelPtr original) {
  threadpool->enqueue([this, original] {
    auto newInstancePtr = modelManager->createModelInstance(original);
    if (!newInstancePtr.has_value()) {
      const auto message = newInstancePtr.error();
      window->enqueue([message, this] {
        ui->imgui->createMsgDlg("Error while creating an instance", "Instance could not be created: {}",
                                Flags{MessageButtons::Ok}, [](auto) { return true; });
      });
    } else {
      auto modelPtr = newInstancePtr.value();
      window->enqueue([modelPtr, this] {
        auto newUIItem = ModelFileInfo{modelPtr->path};
        newUIItem.modelData = modelPtr;
        auto &itemSelectable = ui->activeModelList.addItem(newUIItem);
        addActiveModelPopupMenu(itemSelectable, newUIItem.id, modelPtr);
      });
    }
  });
}

#include <fstream>
void SVORenderer::convertAndSaveSVO(const std::filesystem::path &src, const std::filesystem::path &dir) {
  const auto dst = (dir / src.filename()).replace_extension(".pf_vox");
  logd("CONVERT", "Converting: {} to: {}", src.string(), dst.string());
  const auto svoCreate = vox::loadFileAsSVO(src, true);
  logd("CONVERT", "Voxel count for: {} is: {}", src.string(), svoCreate[0].voxelCount);
  const auto svoBinData = svoCreate[0].data.serialize();
  logd("CONVERT", "Binary size for: {} is: {} bytes", src.string(), svoBinData.size());

  auto ostream = std::ofstream{dst, std::ios::binary};
  const auto materialsSize = toBytes<std::uint32_t>(svoCreate[0].materials.size() * sizeof(vox::MaterialProperties));
  ostream.write(reinterpret_cast<const char *>(materialsSize.data()), materialsSize.size());
  ostream.write(reinterpret_cast<const char *>(svoCreate[0].materials.data()),
                svoCreate[0].materials.size() * sizeof(vox::MaterialProperties));
  ostream.write(reinterpret_cast<const char *>(&svoCreate[0].voxelCount), sizeof(uint32_t));
  ostream.write(reinterpret_cast<const char *>(&svoCreate[0].depth), sizeof(uint32_t));
  const auto aabbData = toBytes(svoCreate[0].AABB);
  ostream.write(reinterpret_cast<const char *>(aabbData.data()), aabbData.size());
  const auto centerData = toBytes(svoCreate[0].center);
  ostream.write(reinterpret_cast<const char *>(centerData.data()), centerData.size());
  ostream.write(reinterpret_cast<const char *>(svoBinData.data()), svoBinData.size());
}
void SVORenderer::createBuffers() {
  cameraUniformBuffer =
      vkLogicalDevice->createBuffer({.size = sizeof(glm::vec4) * 3 + sizeof(glm::mat4) * 3 + 2 * sizeof(float),
                                     .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                                     .sharingMode = vk::SharingMode::eExclusive,
                                     .queueFamilyIndices = {}});

  lightUniformBuffer = vkLogicalDevice->createBuffer({.size = sizeof(glm::vec4) * 4,
                                                      .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                                                      .sharingMode = vk::SharingMode::eExclusive,
                                                      .queueFamilyIndices = {}});

  debugUniformBuffer = vkLogicalDevice->createBuffer({.size = sizeof(uint32_t) * 3 + sizeof(float) + sizeof(float)
                                                          + sizeof(std::uint32_t) + sizeof(std::uint32_t)
                                                          + sizeof(uint32_t) + sizeof(uint32_t),
                                                      .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                                                      .sharingMode = vk::SharingMode::eExclusive,
                                                      .queueFamilyIndices = {}});

  probePosBuffer = vkLogicalDevice->createBuffer({.size = 10_MB,
                                                  .usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
                                                  .sharingMode = vk::SharingMode::eExclusive,
                                                  .queueFamilyIndices = {}});
  probePosBuffer->mapping().set(glm::vec4{0, 0, 0, 0});
  // TODO: size
  svoBuffer = vkLogicalDevice->createBuffer({.size = 500_MB,
                                             .usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
                                             .sharingMode = vk::SharingMode::eExclusive,
                                             .queueFamilyIndices = {}});
  svoMemoryPool = BufferMemoryPool::CreateShared(svoBuffer, 4);
  // TODO: size
  modelInfoBuffer = vkLogicalDevice->createBuffer({.size = 10_MB,
                                                   .usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
                                                   .sharingMode = vk::SharingMode::eExclusive,
                                                   .queueFamilyIndices = {}});

  modelInfoMemoryPool = BufferMemoryPool::CreateShared(modelInfoBuffer, 16);
  // TODO: size
  bvhBuffer = vkLogicalDevice->createBuffer({.size = 10_MB,
                                             .usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
                                             .sharingMode = vk::SharingMode::eExclusive,
                                             .queueFamilyIndices = {}});

  // TODO: size
  materialBuffer = vkLogicalDevice->createBuffer({.size = 10_MB,
                                                  .usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
                                                  .sharingMode = vk::SharingMode::eExclusive,
                                                  .queueFamilyIndices = {}});
  materialMemoryPool = BufferMemoryPool::CreateShared(materialBuffer, 1);
}

}// namespace pf