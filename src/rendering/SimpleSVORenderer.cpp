//
// Created by petr on 1/5/21.
//

#include "SimpleSVORenderer.h"
#include "logging/loggers.h"
#include <fmt/chrono.h>
#include <pf_common/ByteLiterals.h>
#include <pf_imgui/elements.h>
#include <pf_imgui/styles/dark.h>
#include <voxel/SparseVoxelOctreeCreation.h>

namespace pf {
using namespace vulkan;
using namespace pf::byte_literals;

SimpleSVORenderer::SimpleSVORenderer(toml::table &tomlConfig)
    : config(tomlConfig), camera({0, 0}),
      svo(vox::loadFileAsSVO(std::filesystem::path(*config.get()["resources"]["model"].value<std::string>()))) {}

SimpleSVORenderer::~SimpleSVORenderer() {
  if (vkLogicalDevice == nullptr) { return; }
  log(spdlog::level::info, APP_TAG, "Destroying renderer, waiting for device");
  vkLogicalDevice->wait();
  log(spdlog::level::info, APP_TAG, "Saving UI to config");
  imgui->updateConfig();
  config.get()["ui"].as_table()->insert_or_assign("imgui", imgui->getConfig());
}

void SimpleSVORenderer::render() {
  vkSwapChain->swap();

  auto &semaphore = vkSwapChain->getCurrentSemaphore();
  auto &fence = vkSwapChain->getCurrentFence();

  fence.reset();
  vkComputeFence->reset();

  imgui->render();

  recordCommands();

  {
    auto cameraMapping = cameraUniformBuffer->mapping();
    cameraMapping.set(std::vector{glm::vec4{camera.getPosition(), 0}, glm::vec4{camera.getFront(), 0}});
  }

  const auto commandBufferIndex = vkSwapChain->getCurrentImageIndex();
  const auto frameIndex = vkSwapChain->getCurrentFrameIndex();

  vkCommandBuffers[commandBufferIndex]->submit({.waitSemaphores = {semaphore},
                                                   .signalSemaphores = {*computeSemaphore},
                                                   .flags = vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                   .fence = fence,
                                                   .wait = true});

  fence.reset();

  vkGraphicsCommandBuffers[commandBufferIndex]->submit({.waitSemaphores = {*computeSemaphore},
                                                           .signalSemaphores = {*renderSemaphores[frameIndex]},
                                                           .flags = vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                           .fence = fence,
                                                           .wait = true});

  vkSwapChain->present(
      {.waitSemaphores = {*renderSemaphores[frameIndex]}, .presentQueue = vkLogicalDevice->getPresentQueue()});

  vkSwapChain->frameDone();
  fpsCounter.onFrame();
}

void SimpleSVORenderer::createDevices() {
  vkDevice = vkInstance->selectDevice(DefaultDeviceSuitabilityScorer({}, {}, [](const auto &) { return 0; }));
  vkLogicalDevice =
      vkDevice->createLogicalDevice({.id = "dev1",
                                     .deviceFeatures = vk::PhysicalDeviceFeatures{},
                                     .queueTypes = {vk::QueueFlagBits::eCompute, vk::QueueFlagBits::eGraphics},
                                     .presentQueueEnabled = true,
                                     .requiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                                  VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME},
                                     .validationLayers = getValidationLayers(),
                                     .surface = *vkSurface});
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
  /*vkDepthImage = vkLogicalDevice->createImage(
      {.imageType = vk::ImageType::e2D,
       .format = vk::Format::eD32Sfloat,
       .extent =
           vk::Extent3D{.width = vkSwapChain->getExtent().width, .height = vkSwapChain->getExtent().height, .depth = 1},
       .mipLevels = 1,
       .arrayLayers = 1,
       .sampleCount = vk::SampleCountFlagBits::e1,
       .tiling = vk::ImageTiling::eOptimal,
       .usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc
           | vk::ImageUsageFlagBits::eTransferDst
       .sharingQueues = {},
       .layout = vk::ImageLayout::eUndefined});
  vkDepthImageView =
      vkDepthImage->createImageView(vk::ColorSpaceKHR::eSrgbNonlinear, vk::ImageViewType::e2D,
                                    vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});*/
}

void SimpleSVORenderer::createDescriptorPool() {
  vkDescPool = vkLogicalDevice->createDescriptorPool({.flags = {},
                                                      .maxSets = 1,
                                                      .poolSizes = {{vk::DescriptorType::eStorageImage, 1}, // color
                                                                    /*{vk::DescriptorType::eStorageImage, 1}, // depth*/
                                                                    {vk::DescriptorType::eUniformBuffer, 1},// camera
                                                                    {vk::DescriptorType::eUniformBuffer, 1},// light pos
                                                                    {vk::DescriptorType::eStorageBuffer, 1}}});// svo
}

void SimpleSVORenderer::createPipeline() {
  // TODO: compute pipeline builder
  // TODO: descriptor sets
  vkComputeDescSetLayout = vkLogicalDevice->createDescriptorSetLayout(
      {.bindings = {{.binding = 0,
                     .type = vk::DescriptorType::eStorageImage,
                     .count = 1,
                     .stageFlags = vk::ShaderStageFlagBits::eCompute},// color
                  /*  {.binding = 1,
                     .type = vk::DescriptorType::eStorageImage,
                     .count = 1,
                     .stageFlags = vk::ShaderStageFlagBits::eCompute},//depth*/
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
                     .stageFlags = vk::ShaderStageFlagBits::eCompute}}});// svo

  const auto setLayouts = std::vector{**vkComputeDescSetLayout};
  const auto pipelineLayoutInfo =
      vk::PipelineLayoutCreateInfo{.setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
                                   .pSetLayouts = setLayouts.data()};
  auto computePipelineLayout = (*vkLogicalDevice)->createPipelineLayoutUnique(pipelineLayoutInfo);
  auto allocInfo = vk::DescriptorSetAllocateInfo{};
  allocInfo.setSetLayouts(setLayouts);
  allocInfo.descriptorPool = **vkDescPool;
  computeDescriptorSets = (*vkLogicalDevice)->allocateDescriptorSetsUnique(allocInfo);

  cameraUniformBuffer = vkLogicalDevice->createBuffer({.size = sizeof(float) * 8,
                                                       .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                                                       .sharingMode = vk::SharingMode::eExclusive,
                                                       .queueFamilyIndices = {}});

  lightPosUniformBuffer = vkLogicalDevice->createBuffer({.size = sizeof(glm::vec4),
                                                         .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                                                         .sharingMode = vk::SharingMode::eExclusive,
                                                         .queueFamilyIndices = {}});

  // TODO: size
  svoBuffer = vkLogicalDevice->createBuffer({.size = 100_MB,
                                             .usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
                                             .sharingMode = vk::SharingMode::eExclusive,
                                             .queueFamilyIndices = {}});

  // TODO: svo storage buffer

  const auto computeColorInfo = vk::DescriptorImageInfo{.sampler = {},
                                                        .imageView = **vkRenderImageView,
                                                        .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeColorWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                        .dstBinding = 0,
                                                        .dstArrayElement = {},
                                                        .descriptorCount = 1,
                                                        .descriptorType = vk::DescriptorType::eStorageImage,
                                                        .pImageInfo = &computeColorInfo};
  /*const auto computeDepthInfo =
      vk::DescriptorImageInfo{.sampler = {}, .imageView = **vkDepthImageView, .imageLayout = vk::ImageLayout::eGeneral};

  const auto computeDepthWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                        .dstBinding = 1,
                                                        .dstArrayElement = {},
                                                        .descriptorCount = 1,
                                                        .descriptorType = vk::DescriptorType::eStorageImage,
                                                        .pImageInfo = &computeDepthInfo};*/

  const auto uniformCameraInfo =
      vk::DescriptorBufferInfo{.buffer = **cameraUniformBuffer, .offset = 0, .range = cameraUniformBuffer->getSize()};
  const auto uniformCameraWrite = vk::WriteDescriptorSet{.dstSet = *computeDescriptorSets[0],
                                                         .dstBinding = 1,
                                                         .dstArrayElement = {},
                                                         .descriptorCount = 1,
                                                         .descriptorType = vk::DescriptorType::eUniformBuffer,
                                                         .pBufferInfo = &uniformCameraInfo};

  const auto lightPosInfo = vk::DescriptorBufferInfo{.buffer = **lightPosUniformBuffer,
                                                     .offset = 0,
                                                     .range = lightPosUniformBuffer->getSize()};
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

  const auto writeSets = std::vector{computeColorWrite/*, computeDepthWrite*/, uniformCameraWrite, lightPosWrite, svoWrite};
  (*vkLogicalDevice)->updateDescriptorSets(writeSets, nullptr);

  // TODO: change string paths to filesystem::path
  auto computeShader = vkLogicalDevice->createShader(ShaderConfigGlslFile{
      .name = "Triangle compute",
      .type = ShaderType::Compute,
      .path = (std::filesystem::path(*config.get()["resources"]["path_shaders"].value<std::string>()) /= "svo.comp")
                  .string(),
      .macros = {},
      .replaceMacros = {}});

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
      recording.dispatch(vkSwapChain->getExtent().width / 8, vkSwapChain->getExtent().height / 8, 1);
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
  imgui->addToCommandBuffer(graphRecording);
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

  setDarkStyle(*imgui);

  auto &debugWindow = imgui->createChild<Window>("debug_window", "Debug");

  auto &debugWindowTabs = debugWindow.createChild<TabBar>("debug_tabbar");
  auto &chaiTab = debugWindowTabs.addTab("chai_tab", "ChaiScript");
  auto &logTab = debugWindowTabs.addTab("log_tab", "Log");

  auto &logMemo = logTab.createChild<Memo>("log_output", "Log:", 100, true, true, 100);
  addLogListener([&logMemo = logMemo](auto record) { logMemo.addRecord(record); });
  auto &logErrMemo = logTab.createChild<Memo>("log_err_output", "Log err:", 100, true, true, 100);
  addLogListener([&logErrMemo = logErrMemo](auto record) { logErrMemo.addRecord(record); }, true);

  auto &chaiInputPanel =
      chaiTab.createChild<Panel>("chai_input_panel", "Input", PanelLayout::Horizontal, ImVec2{0, 50});

  chaiInputPanel.createChild<Text>("chain_input_label", "Input:");
  auto &chaiInput = chaiInputPanel.createChild<InputText>("chai_input", "", "", TextInputType::MultiLine);
  auto &chai_output = chaiTab.createChild<Memo>("chai_output", "Output:", 100, true, true, 100);

  chaiInputPanel.createChild<Button>("chain_input_confirm", "Confirm")
      .addClickListener([&chaiInput = chaiInput, &chai_output = chai_output, this] {
        const auto input = chaiInput.getText();
        chai_output.addRecord(">>> "s + input);
        chaiInput.clear();
        try {
          chai->eval(input);
        } catch (const chaiscript::exception::eval_error &e) { chai_output.addRecord("<<< "s + e.pretty_print()); }
      });

  chai->add(chaiscript::fun([](const std::string &str) { log(spdlog::level::debug, APP_TAG, str); }), "log");

  auto &infoWindow = imgui->createChild<Window>("infoWindow", "Stats");

  auto fpsPlot = &infoWindow.createChild<SimplePlot>("fps_plot", "Fps", PlotType::Histogram, std::vector<float>{},
                                                     std::nullopt, 200, 0, 60, ImVec2{0, 50});
  const auto fpsMsgTemplate = "FPS:\nCurrent: {0:0.2f}\nAverage: {0:0.2f}";
  auto fpsLabel = &infoWindow.createChild<Text>("fpsText", "FPS");
  infoWindow.createChild<Button>("fpsResetBtn", "Reset FPS").addClickListener([this] { fpsCounter.reset(); });

  infoWindow.createChild<Checkbox>("vsync_chckbx", "Enable vsync", Persistent::Yes, true)
      .addValueListener([](auto enabled) { logdFmt("UI", "Vsync enabled: {}", enabled); });

  infoWindow
      .createChild<Slider<glm::vec3>>("slider_lightpos", "Light position", -10, 10, glm::vec3{0, -2, 0},
                                      Persistent::Yes)
      .addValueListener([&](const auto &pos) {
        lightPosUniformBuffer->mapping().set(std::span{&pos, 1});
      });

  auto &cameraGroup = infoWindow.createChild<Group>("cameraGroup", "Camera");
  const auto cameraPosTemplate = "Position: {0:0.2f}x{1:0.2f}x{2:0.2f}";
  const auto cameraDirTemplate = "Direction: {0:0.2f}x{1:0.2f}x{2:0.2f}";
  auto cameraPosText = &cameraGroup.createChild<Text>("cameraPositionText", "");
  auto cameraDirText = &cameraGroup.createChild<Text>("cameraDirText", "");
  cameraGroup
      .createChild<Slider<float>>("cameraMoveSpeedSlider", "Movement speed", 0.1f, 50.f, camera.getMovementSpeed(),
                                  Persistent::Yes)
      .addValueListener([this](auto value) { camera.setMovementSpeed(value); });
  cameraGroup
      .createChild<Slider<float>>("cameraMouseSpeedSlider", "Mouse speed", 0.1f, 50.f, camera.getMouseSpeed(),
                                  Persistent::Yes)
      .addValueListener([this](auto value) { camera.setMouseSpeed(value); });
  cameraGroup
      .createChild<Slider<float>>("cameraFOVSlider", "Field of view", 1.f, 90.f, camera.getFieldOfView(),
                                  Persistent::Yes)
      .addValueListener([this](auto value) { camera.setFieldOfView(value); });

  fpsCounter.setOnNewFrame([this, cameraPosText, cameraDirText, cameraDirTemplate, cameraPosTemplate, fpsPlot,
                            fpsMsgTemplate, fpsLabel](const FPSCounter &counter) {
    fpsPlot->addValue(counter.currentFPS());
    fpsLabel->setText(fmt::format(fpsMsgTemplate, counter.currentFPS(), counter.averageFPS()));
    const auto camPos = camera.getPosition();
    cameraPosText->setText(fmt::format(cameraPosTemplate, camPos.x, camPos.y, camPos.z));
    const auto camDir = camera.getFront();
    cameraDirText->setText(fmt::format(cameraDirTemplate, camDir.x, camDir.y, camDir.z));
  });

  imgui->setStateFromConfig();
}

}// namespace pf