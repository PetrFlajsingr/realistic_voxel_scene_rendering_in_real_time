//
// Created by petr on 1/5/21.
//

#include "SimpleSVORenderer.h"
#include "../../../pf_common/include/pf_common/files.h"
#include "logging/loggers.h"
#include <fmt/chrono.h>
#include <pf_common/ByteLiterals.h>
#include <pf_imgui/elements.h>
#include <pf_imgui/styles/dark.h>
#include <voxel/SVO_utils.h>
#include <voxel/SparseVoxelOctreeCreation.h>

namespace pf {
using namespace vulkan;
using namespace pf::byte_literals;

SimpleSVORenderer::SimpleSVORenderer(toml::table &tomlConfig)
    : config(tomlConfig), camera({0, 0}, 2.5, 2.5, {1.4, 0.8, 2.24}) {}

SimpleSVORenderer::~SimpleSVORenderer() {
  if (vkLogicalDevice == nullptr) { return; }
  log(spdlog::level::info, APP_TAG, "Destroying renderer, waiting for device");
  vkLogicalDevice->wait();
  log(spdlog::level::info, APP_TAG, "Saving UI to config");
  imgui->updateConfig();
  config.get()["ui"].as_table()->insert_or_assign("imgui", imgui->getConfig());
}

void SimpleSVORenderer::render() {
  if (!isSceneLoaded) {
    isSceneLoaded = true;
    vox::copySvoToBuffer(*svo, svoBuffer->mapping());
    logd(APP_TAG, "Copied svo to buffer");
  }

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
                                                       .stageFlags = vk::ShaderStageFlagBits::eCompute},// svo
                                                      {.binding = 4,
                                                       .type = vk::DescriptorType::eUniformBuffer,
                                                       .count = 1,
                                                       .stageFlags = vk::ShaderStageFlagBits::eCompute},// debug
                                                      {.binding = 5,
                                                       .type = vk::DescriptorType::eStorageImage,
                                                       .count = 1,
                                                       .stageFlags = vk::ShaderStageFlagBits::eCompute},// iter
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

  cameraUniformBuffer = vkLogicalDevice->createBuffer({.size = sizeof(float) * 8,
                                                       .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                                                       .sharingMode = vk::SharingMode::eExclusive,
                                                       .queueFamilyIndices = {}});

  lightUniformBuffer = vkLogicalDevice->createBuffer({.size = sizeof(glm::vec4) * 4,
                                                      .usageFlags = vk::BufferUsageFlagBits::eUniformBuffer,
                                                      .sharingMode = vk::SharingMode::eExclusive,
                                                      .queueFamilyIndices = {}});

  // TODO: size
  svoBuffer = vkLogicalDevice->createBuffer({.size = 100_MB,
                                             .usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
                                             .sharingMode = vk::SharingMode::eExclusive,
                                             .queueFamilyIndices = {}});

  debugUniformBuffer = vkLogicalDevice->createBuffer({.size = sizeof(uint32_t) * 3,
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

  const auto writeSets =
      std::vector{computeColorWrite, uniformCameraWrite, lightPosWrite, svoWrite, uniformDebugWrite, computeIterWrite};
  (*vkLogicalDevice)->updateDescriptorSets(writeSets, nullptr);

  // TODO: change string paths to filesystem::path
  auto computeShader = vkLogicalDevice->createShader(ShaderConfigGlslFile{
      .name = "Triangle compute",
      .type = ShaderType::Compute,
      .path = (std::filesystem::path(*config.get()["resources"]["path_shaders"].value<std::string>()) /= "svo.comp")
                  .string(),
      .macros = {},
      .replaceMacros = {{"LOCAL_SIZE_X", std::to_string(LOCAL_SIZE_X)},
                        {"LOCAL_SIZE_Y", std::to_string(LOCAL_SIZE_Y)}}});

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
      recording.dispatch(vkSwapChain->getExtent().width / LOCAL_SIZE_X, vkSwapChain->getExtent().height / LOCAL_SIZE_Y,
                         1);
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

  auto &renderSettingsWindow = imgui->createChild<Window>("render_sett_window", "Render settings");
  renderSettingsWindow
      .createChild<ComboBox>(
          "view_choice", "View type", "Select view type",
          std::vector<std::string>{"Color", "Normals", "Iterations", "Distance", "Child index", "Tree level"},
          Persistent::Yes)
      .addValueListener([this](const auto &viewType) {
        auto viewTypeIdx = 0;
        if (viewType == "Color") {
          viewTypeIdx = 0;
        } else if (viewType == "Normals") {
          viewTypeIdx = 1;
        } else if (viewType == "Iterations") {
          viewTypeIdx = 2;
        } else if (viewType == "Distance") {
          viewTypeIdx = 3;
        } else if (viewType == "Child index") {
          viewTypeIdx = 4;
        } else if (viewType == "Tree level") {
          viewTypeIdx = 5;
        }
        debugUniformBuffer->mapping().set(viewTypeIdx);
      });
  auto &lightGroup = renderSettingsWindow.createChild<Group>("light_group", "Lighting");
  lightGroup
      .createChild<Slider<glm::vec3>>("slider_lightpos", "Light position", -100, 100, glm::vec3{0, -2, 0},
                                      Persistent::Yes)
      .addValueListener([&](const auto &pos) { lightUniformBuffer->mapping().set(pos); });
  lightGroup.createChild<Checkbox>("check_shadows", "Enable shadows", Persistent::Yes, false)
      .addValueListener([this](auto enabled) { debugUniformBuffer->mapping().set(enabled ? 1 : 0, 2); });

  auto &phongParams = lightGroup.createChild<Group>("phong_params_group", "Phong parameters");
  auto &ambientSlider = phongParams.createChild<Slider<glm::vec3>>("slider_light_ambient", "Ambient", 0, 1,
                                                                   glm::vec3{0.1f}, Persistent::Yes);
  ambientSlider.addValueListener([&](const auto &ambientColor) {
    lightUniformBuffer->mapping().set(glm::vec4{ambientColor, 1}, 1);
  });
  phongParams.createChild<Slider<glm::vec3>>("slider_light_diffuse", "Diffuse", 0, 1, glm::vec3{0.6f}, Persistent::Yes)
      .addValueListener([&](const auto &diffuseColor) {
        lightUniformBuffer->mapping().set(glm::vec4{diffuseColor, 1}, 2);
      });
  phongParams
      .createChild<Slider<glm::vec3>>("slider_light_specular", "Specular", 0, 1, glm::vec3{0.9f}, Persistent::Yes)
      .addValueListener([&](const auto &specularColor) {
        lightUniformBuffer->mapping().set(glm::vec4{specularColor, 1}, 3);
      });

  auto &modelsGroup = renderSettingsWindow.createChild<Group>("models_group", "Models");

  const auto modelsPath = std::filesystem::path(*config.get()["resources"]["path_models"].value<std::string>());
  const auto modelFileNames = loadModelFileNames(modelsPath);

  modelsGroup.createChild<Button>("open_model_btn", "Open model").addClickListener([this] {
    imgui->openFileDialog(
        "Select model", {FileExtensionSettings{{"vox"}, "Vox model"}},
        [this](const auto &selected) {
          svo = std::make_unique<vox::SparseVoxelOctree>(vox::loadFileAsSVO(selected[0]));
          isSceneLoaded = false;
        },
        [] {});
  });

  auto &modelCB =
      modelsGroup.createChild<ComboBox>("models_choice", "Model", "Select model", modelFileNames, Persistent::Yes);
  modelCB.addValueListener([modelsPath, this](const auto &modelName) {
    const auto selectedModelPath = modelsPath / modelName;
    svo = std::make_unique<vox::SparseVoxelOctree>(vox::loadFileAsSVO(selectedModelPath));
    isSceneLoaded = false;
  });
  modelsGroup.createChild<InputText>("model_filter", "Filter").addValueListener([&modelCB](auto filterVal) {
    modelCB.setFilter([filterVal](auto item) { return item.find(filterVal) != std::string::npos; });
  });

  modelsGroup.createChild<Button>("model_list_reload", "Reload models").addClickListener([&modelCB, modelsPath, this] {
    const auto modelFileNames = loadModelFileNames(modelsPath);
    modelCB.setItems(modelFileNames);
  });
  modelsGroup.createChild<Button>("model_reload", "Reload selected").addClickListener([&modelCB, modelsPath, this] {
    if (const auto selected = modelCB.getSelectedItem(); selected.has_value()) {
      const auto selectedModelPath = modelsPath / *modelCB.getSelectedItem();
      svo = std::make_unique<vox::SparseVoxelOctree>(vox::loadFileAsSVO(selectedModelPath));
      isSceneLoaded = false;
    }
  });

  auto &debugWindow = imgui->createChild<Window>("debug_window", "Debug");
  debugWindow.createChild<Input<int>>("debug_val_inpug", "Shader debug value", -100000, 100000, Persistent::Yes, 0)
      .addValueListener([this](const auto &val) { debugUniformBuffer->mapping().set(val, 1); });

  auto &debugWindowTabs = debugWindow.createChild<TabBar>("debug_tabbar");
  auto &logTab = debugWindowTabs.addTab("log_tab", "Log");
  auto &chaiTab = debugWindowTabs.addTab("chai_tab", "ChaiScript");

  auto &logMemo = logTab.createChild<Memo>("log_output", "Log:", 100, true, true, 100);
  subscriptions.emplace_back(addLogListener([&logMemo = logMemo](auto record) { logMemo.addRecord(record); }));
  auto &logErrMemo = logTab.createChild<Memo>("log_err_output", "Log err:", 100, true, true, 100);
  subscriptions.emplace_back(
      addLogListener([&logErrMemo = logErrMemo](auto record) { logErrMemo.addRecord(record); }, true));

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

  auto &debugImagesWindow = imgui->createChild<Window>("debug_images_window", "Debug images");
  const auto extent = vkSwapChain->getExtent();
  const auto imageWidth = 400.f;
  const auto imageHeight = static_cast<float>(extent.height) / extent.width * imageWidth;
  debugImagesWindow.createChild<pf::ui::ig::Image>(
      "iter_image",
      (ImTextureID) ImGui_ImplVulkan_AddTexture(**vkIterImageSampler, **vkIterImageView,
                                                static_cast<VkImageLayout>(vkIterImage->getLayout())),
      ImVec2{imageWidth, imageHeight});

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
std::vector<std::string> SimpleSVORenderer::loadModelFileNames(const std::filesystem::path &dir) {
  const auto potentialModelFiles = filesInFolder(dir);
  return potentialModelFiles | ranges::views::filter([](const auto &path) { return path.extension() == ".vox"; })
      | ranges::views::transform([](const auto &path) { return path.filename().string(); }) | ranges::to_vector
      | ranges::actions::sort;
}
void SimpleSVORenderer::stop() {
  for (auto &subscription : subscriptions) { subscription.unsubscribe(); }
}

}// namespace pf