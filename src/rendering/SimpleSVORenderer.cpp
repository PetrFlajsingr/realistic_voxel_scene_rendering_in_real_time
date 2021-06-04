//
// Created by petr on 1/5/21.
//

#include "SimpleSVORenderer.h"
#include "logging/loggers.h"
#include <experimental/array>
#include <fmt/chrono.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <pf_common/ByteLiterals.h>
#include <pf_common/enums.h>
#include <pf_common/files.h>
#include <pf_glfw_vulkan/ui/GlfwWindow.h>
#include <pf_imgui/elements.h>
#include <pf_imgui/interface/decorators/WidthDecorator.h>
#include <pf_imgui/styles/dark.h>
#include <pf_imgui/unique_id.h>
#include <utils/FlameGraphSampler.h>
#include <voxel/SVO_utils.h>
#include <voxel/SparseVoxelOctreeCreation.h>

namespace pf {
using namespace vulkan;
using namespace pf::byte_literals;

// TODO: remove this and make it work from pf_common/enums.h
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

SimpleSVORenderer::SimpleSVORenderer(toml::table &tomlConfig)
    : config(tomlConfig), camera({0, 0}, 0.001f, 50.f, 2.5, 2.5, {1.4, 0.8, 2.24}) {
  computeLocalSize = std::pair{config.get()["rendering"]["compute"]["local_size_x"].value_or<std::size_t>(8),
                               config.get()["rendering"]["compute"]["local_size_y"].value_or<std::size_t>(8)};
}

SimpleSVORenderer::~SimpleSVORenderer() {
  if (vkLogicalDevice == nullptr) { return; }
  log(spdlog::level::info, APP_TAG, "Destroying renderer, waiting for device");
  vkLogicalDevice->wait();
  log(spdlog::level::info, APP_TAG, "Saving UI to config");
  ui->imgui->updateConfig();
  config.get()["ui"].as_table()->insert_or_assign("imgui", ui->imgui->getConfig());
}

void SimpleSVORenderer::init(const std::shared_ptr<ui::Window> &win) {
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
  auto imgui = std::make_unique<ui::ig::ImGuiGlfwVulkan>(vkLogicalDevice, vkRenderPass, vkSurface, vkSwapChain,
                                                         windowHandle, ImGuiConfigFlags{}, imguiConfig);
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

  ui = std::make_unique<SimpleSVORenderer_UI>(std::move(imgui), camera,
                                              TextureData{*vkIterImage, *vkIterImageView, *vkIterImageSampler});

  initUI();
  window->setMainLoopCallback([&] { render(); });
}

std::unordered_set<std::string> SimpleSVORenderer::getValidationLayers() {
  return std::unordered_set<std::string>{"VK_LAYER_KHRONOS_validation"};
}

void SimpleSVORenderer::buildVulkanObjects() {
  createSwapchain();
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

void SimpleSVORenderer::createInstance() {
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

void SimpleSVORenderer::createSurface() { vkSurface = vkInstance->createSurface(window); }

void SimpleSVORenderer::render() {
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
    const auto invProjView = glm::inverse(camera.getProjectionMatrix() * camera.getViewMatrix());
    cameraMapping.setRawOffset(invProjView, sizeof(glm::vec4) * 3 + sizeof(glm::mat4) * 2);
    cameraMapping.setRawOffset(camera.getNear(), sizeof(glm::vec4) * 3 + sizeof(glm::mat4) * 3);
    cameraMapping.setRawOffset(camera.getFar(), sizeof(glm::vec4) * 3 + sizeof(glm::mat4) * 3 + sizeof(float));
  }

  const auto commandBufferIndex = vkSwapChain->getCurrentImageIndex();
  const auto frameIndex = vkSwapChain->getCurrentFrameIndex();

  auto computeSample = mainSample.blockSampler("compute");
  vkCommandBuffers[commandBufferIndex]->submit({.waitSemaphores = {semaphore},
                                                .signalSemaphores = {*computeSemaphore},
                                                .flags = vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                .fence = fence,
                                                .wait = true});

  fence.reset();
  computeSample.end();

  auto presentSample = mainSample.blockSampler("present");
  vkGraphicsCommandBuffers[commandBufferIndex]->submit({.waitSemaphores = {*computeSemaphore},
                                                        .signalSemaphores = {*renderSemaphores[frameIndex]},
                                                        .flags = vk::PipelineStageFlagBits::eColorAttachmentOutput,
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

void SimpleSVORenderer::createDevices() {
  vkDevice = vkInstance->selectDevice(DefaultDeviceSuitabilityScorer({}, {}, [](const auto &) { return 0; }));
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

void SimpleSVORenderer::createSwapchain() {
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

void SimpleSVORenderer::createRenderTextures() {
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

void SimpleSVORenderer::createDescriptorPool() {
  vkDescPool = vkLogicalDevice->createDescriptorPool({.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                                      .maxSets = 1,
                                                      .poolSizes = {
                                                          {vk::DescriptorType::eStorageImage, 1}, // color
                                                          {vk::DescriptorType::eUniformBuffer, 1},// camera
                                                          {vk::DescriptorType::eUniformBuffer, 1},// light pos
                                                          {vk::DescriptorType::eStorageBuffer, 1},// svo
                                                          {vk::DescriptorType::eUniformBuffer, 1},// view type
                                                          {vk::DescriptorType::eStorageImage, 1}, // iterations
                                                          {vk::DescriptorType::eUniformBuffer, 1},// matrices
                                                      }});
}

void SimpleSVORenderer::createPipeline() {
  // TODO: compute pipeline builder
  // TODO: descriptor sets
  vkComputeDescSetLayout =
      vkLogicalDevice->createDescriptorSetLayout({.bindings = {
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

  cameraUniformBuffer =
      vkLogicalDevice->createBuffer({.size = sizeof(glm::vec4) * 3 + sizeof(glm::mat4) * 3 + 2 * sizeof(float),
                                     .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                                     .sharingMode = vk::SharingMode::eExclusive,
                                     .queueFamilyIndices = {}});

  lightUniformBuffer = vkLogicalDevice->createBuffer({.size = sizeof(glm::vec4) * 4,
                                                      .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                                                      .sharingMode = vk::SharingMode::eExclusive,
                                                      .queueFamilyIndices = {}});

  // TODO: size
  svoBuffer = vkLogicalDevice->createBuffer({.size = 500_MB,
                                             .usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
                                             .sharingMode = vk::SharingMode::eExclusive,
                                             .queueFamilyIndices = {}});
  svoMemoryPool = BufferMemoryPool<4>::CreateShared(svoBuffer);
  // TODO: size
  modelInfoBuffer = vkLogicalDevice->createBuffer({.size = 10_MB,
                                                   .usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
                                                   .sharingMode = vk::SharingMode::eExclusive,
                                                   .queueFamilyIndices = {}});

  modelInfoMemoryPool = BufferMemoryPool<16>::CreateShared(modelInfoBuffer);
  // TODO: size
  bvhBuffer = vkLogicalDevice->createBuffer({.size = 10_MB,
                                             .usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
                                             .sharingMode = vk::SharingMode::eExclusive,
                                             .queueFamilyIndices = {}});

  debugUniformBuffer = vkLogicalDevice->createBuffer(
      {.size = sizeof(uint32_t) * 3 + sizeof(float) + sizeof(float) + sizeof(std::uint32_t) + sizeof(std::uint32_t),
       .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
       .sharingMode = vk::SharingMode::eExclusive,
       .queueFamilyIndices = {}});

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

  const auto writeSets = std::vector{computeColorWrite, uniformCameraWrite, lightPosWrite,  svoWrite,
                                     uniformDebugWrite, computeIterWrite,   modelInfoWrite, bvhWrite};
  (*vkLogicalDevice)->updateDescriptorSets(writeSets, nullptr);

  // TODO: change string paths to filesystem::path
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
void SimpleSVORenderer::createCommands() {
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
void SimpleSVORenderer::recordCommands() {
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
  ui->imgui->addToCommandBuffer(graphRecording);
  graphRecording.endRenderPass();
}
void SimpleSVORenderer::createFences() {
  vkComputeFence = vkLogicalDevice->createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
  std::ranges::generate_n(std::back_inserter(fences), vkSwapChain->getFrameBuffers().size(),
                          [&] { return vkLogicalDevice->createFence({.flags = vk::FenceCreateFlagBits::eSignaled}); });
}
void SimpleSVORenderer::createSemaphores() {
  computeSemaphore = vkLogicalDevice->createSemaphore();
  std::ranges::generate_n(std::back_inserter(renderSemaphores), vkSwapChain->getFrameBuffers().size(),
                          [&] { return vkLogicalDevice->createSemaphore(); });
}
void SimpleSVORenderer::initUI() {
  using namespace std::string_literals;
  using namespace pf::ui::ig;
  using namespace pf::enum_operators;

  //auto openModelFnc = [this] {
  //  ui->imgui->openFileDialog(
  //      "Select model", {FileExtensionSettings{{"vox"}, "Vox model", ImVec4{1, 0, 0, 1}}},
  //      [this](const auto &selected) {
  //        auto modelPath = selected[0];
  //        const auto svoCreate = vox::loadFileAsSVO(modelPath);
  //        // TODO: add model to active model list
  //        svo = std::make_unique<vox::SparseVoxelOctree>(std::move(svoCreate.first));
  //        isSceneLoaded = false;
  //      },
  //      [] {});
  //};

  ui->closeMenuItem.addClickListener(closeWindow);
  // TODO: ui->openModelMenuItem.addClickListener(openModelFnc);

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

  ui->modelDetailTranslateDrag.addValueListener([this](const auto &) {
    if (auto item = ui->activeModelList.getSelectedItem(); item.has_value()) {
      item->get().updateInfoToGPU();
      rebuildAndUploadBVH();
    }
  });
  ui->modelDetailScaleDrag.addValueListener([this](const auto &) {
    if (auto item = ui->activeModelList.getSelectedItem(); item.has_value()) {
      item->get().updateInfoToGPU();
      rebuildAndUploadBVH();
    }
  });
  ui->modelDetailRotateDrag.addValueListener(
      [this](const auto &) {
        if (auto item = ui->activeModelList.getSelectedItem(); item.has_value()) {
          item->get().updateInfoToGPU();
          rebuildAndUploadBVH();
        }
      },
      true);

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
  ui->modelList.setItems(modelFileNames
                         | std::views::transform([](const auto &path) { return vox::GPUModelInfo{path}; }));

  //ui->openModelButton.addClickListener(openModelFnc);

  ui->activeModelList.addDropListener([this, modelsPath](const auto &modelInfo) {
    const auto selectedModelPath = modelsPath / modelInfo.path;
    const auto svoCreate = vox::loadFileAsSVO(selectedModelPath);
    auto newModelInfo = vox::GPUModelInfo{};
    newModelInfo.path = modelInfo.path;
    newModelInfo.voxelCount = svoCreate.second.initVoxelCount;
    newModelInfo.minimizedVoxelCount = svoCreate.second.voxelCount;
    newModelInfo.svoHeight = svoCreate.second.depth;
    newModelInfo.AABB = svoCreate.second.AABB;
    auto svo = vox::SparseVoxelOctree{std::move(svoCreate.first)};
    auto svoBlockResult = copySvoToMemoryBlock(svo, *svoMemoryPool);

    auto modelInfoBlockResult = modelInfoMemoryPool->leaseMemory(vox::MODEL_INFO_BLOCK_SIZE);
    std::string err;
    if (!modelInfoBlockResult.has_value()) { err += modelInfoBlockResult.error(); }
    if (!svoBlockResult.has_value()) { err += modelInfoBlockResult.error(); }
    if (!err.empty()) {
      loge(MAIN_TAG, "Error while loading SVO: {}", err);
      std::cout << MessageButtons::Ok;
      ui->imgui->createMsgDlg<MessageButtons>("Error", fmt::format("Error while loading SVO: {}", err),
                                              MessageButtons::Ok, [](auto) { return true; });
      window->enqueue([modelInfo, this] { ui->activeModelList.removeItem(modelInfo); });
      return;
    }

    newModelInfo.svoMemoryBlock = std::make_shared<BufferMemoryPool<4>::Block>(std::move(*svoBlockResult));
    newModelInfo.modelInfoMemoryBlock = std::make_shared<BufferMemoryPool<16>::Block>(std::move(*modelInfoBlockResult));
    newModelInfo.updateInfoToGPU();

    window->enqueue([modelInfo, newModelInfo, this] {
      ui->activeModelList.removeItem(modelInfo);
      ui->activeModelList.addItem(newModelInfo, Selected::Yes);
      rebuildAndUploadBVH();
    });
  });// TODO: change this to load new SVO not reload

  ui->activateSelectedModelButton.addClickListener([this, modelsPath] {
    if (auto item = ui->modelList.getSelectedItem(); item.has_value()) {
      auto newItem = item->get();
      const auto selectedModelPath = modelsPath / newItem.path;
      const auto svoCreate = vox::loadFileAsSVO(selectedModelPath);
      newItem.assignNewId();
      newItem.voxelCount = svoCreate.second.initVoxelCount;
      newItem.minimizedVoxelCount = svoCreate.second.voxelCount;
      newItem.svoHeight = svoCreate.second.depth;
      newItem.AABB = svoCreate.second.AABB;

      auto svo = vox::SparseVoxelOctree{std::move(svoCreate.first)};
      auto svoBlockResult = copySvoToMemoryBlock(svo, *svoMemoryPool);

      auto modelInfoBlockResult = modelInfoMemoryPool->leaseMemory(vox::MODEL_INFO_BLOCK_SIZE);
      std::string err;
      if (!modelInfoBlockResult.has_value()) { err += modelInfoBlockResult.error(); }
      if (!svoBlockResult.has_value()) { err += modelInfoBlockResult.error(); }
      if (!err.empty()) {
        loge(MAIN_TAG, "Error while loading SVO: {}", err);
        std::cout << MessageButtons::Ok;
        ui->imgui->createMsgDlg<MessageButtons>("Error", fmt::format("Error while loading SVO: {}", err),
                                                MessageButtons::Ok, [](auto) { return true; });
        return;
      }

      newItem.svoMemoryBlock = std::make_shared<BufferMemoryPool<4>::Block>(std::move(*svoBlockResult));
      newItem.modelInfoMemoryBlock = std::make_shared<BufferMemoryPool<16>::Block>(std::move(*modelInfoBlockResult));
      newItem.updateInfoToGPU();
      ui->activeModelList.addItem(newItem, Selected::Yes);
      rebuildAndUploadBVH();
    }
  });// TODO: change so this will load new SVO not reload

  ui->modelsFilterInput.addValueListener([this](auto filterVal) {
    ui->modelList.setFilter(
        [filterVal](const auto &item) { return toString(item).find(filterVal) != std::string::npos; });
  });

  ui->reloadModelListButton.addClickListener([modelsPath, this] {
    const auto modelFileNames = loadModelFileNames(modelsPath);
    ui->modelList.setItems(modelFileNames
                           | std::views::transform([](const auto &path) { return vox::GPUModelInfo{path}; }));
  });

  ui->shaderDebugValueInput.addValueListener([this](const auto &val) { debugUniformBuffer->mapping().set(val, 1); },
                                             true);

  subscriptions.emplace_back(addLogListener([this](auto record) { ui->logMemo.addRecord(record); }));
  subscriptions.emplace_back(addLogListener([this](auto record) { ui->logErrMemo.addRecord(record); }, true));

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

  //auto &debugImagesWindow = imgui->createWindow("debug_images_window", "Debug images");
  //renderSettingsWindow
  //    .createChild<ComboBox>(
  //        "view_choice_texture", "View type", "Select view type",
  //        std::vector<std::string>{"Color", "Normals", "Iterations", "Distance", "Child index", "Tree level"},
  //        Persistent::Yes)
  //    .addValueListener(
  //        [](const auto &) {
  //          logd(MAIN_TAG, "TODO: view_choice_texture");
  //        },
  //        true);

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

  ui->removeSelectedActiveModelButton.addClickListener([this] {
    if (!ui->activeModelList.getSelectedItem().has_value()) { return; }
    const auto idToRemove = ui->activeModelList.getSelectedItem()->get().id;
    ui->activeModelList.removeItemIf([idToRemove](const auto &modelInfo) { return modelInfo.id == idToRemove; });
  });

  ui->debugPrintEnableCheckbox.addValueListener(
      [this](auto enabled) { debugUniformBuffer->mapping(sizeof(std::uint32_t) * 5).set(enabled ? 1 : 0); }, true);

  ui->bvhVisualizeCheckbox.addValueListener(
      [this](auto enabled) { debugUniformBuffer->mapping(sizeof(std::uint32_t) * 6).set(enabled ? 1 : 0); }, true);

  ui->imgui->setStateFromConfig();
}
std::vector<std::string> SimpleSVORenderer::loadModelFileNames(const std::filesystem::path &dir) {
  const auto potentialModelFiles = filesInFolder(dir);
  return potentialModelFiles | ranges::views::filter([](const auto &path) { return path.extension() == ".vox"; })
      | ranges::views::transform([](const auto &path) { return path.filename().string(); }) | ranges::to_vector
      | ranges::actions::sort;
}
void SimpleSVORenderer::stop() {
  for (auto &subscription : subscriptions) { subscription.unsubscribe(); }
}
void SimpleSVORenderer::rebuildAndUploadBVH() {
  auto bvhTree = vox::createBVH(ui->activeModelList.getItems());
  auto mapping = bvhBuffer->mapping();
  vox::saveBVHToBuffer(bvhTree, mapping);
}

}// namespace pf