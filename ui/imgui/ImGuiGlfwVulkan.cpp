//
// Created by petr on 10/27/20.
//

#include "ImGuiGlfwVulkan.h"
#include "../vulkan/types/types.h"
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

namespace pf::ui {

namespace details {
void checkVkResult(VkResult err) {
  if (err == 0) { return; }
  fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
  if (err < 0) { throw vulkan::VulkanException::fmt("Error: VkResult = {}", err); }
}
}// namespace details

ImGuiGlfwVulkan::ImGuiGlfwVulkan(std::shared_ptr<vulkan::LogicalDevice> device,
                                 std::shared_ptr<vulkan::RenderPass> pass,
                                 std::shared_ptr<vulkan::Surface> surf,
                                 std::shared_ptr<vulkan::SwapChain> swapCh, GLFWwindow *handle,
                                 ImGuiConfigFlags flags)
    : ImGuiInterface(flags), logicalDevice(std::move(device)), renderPass(std::move(pass)),
      surface(std::move(surf)), swapChain(std::move(swapCh)) {

  auto &physicalDevice = logicalDevice->getPhysicalDevice();
  const auto imageCount = swapChain->getImageViews().size();

  setupDescriptorPool();
  ImGui_ImplGlfw_InitForVulkan(handle, true);
  auto init_info = ImGui_ImplVulkan_InitInfo();
  init_info.Instance = *physicalDevice.getInstance();
  init_info.PhysicalDevice = *physicalDevice;
  init_info.Device = **logicalDevice;
  init_info.QueueFamily = logicalDevice->getQueueIndices()[vk::QueueFlagBits::eGraphics];
  init_info.Queue = logicalDevice->getPresentQueue();
  init_info.PipelineCache = VK_NULL_HANDLE;
  init_info.DescriptorPool = **descriptorPool;
  init_info.Allocator = nullptr;
  init_info.MinImageCount = imageCount;
  init_info.ImageCount = imageCount;
  init_info.CheckVkResultFn = details::checkVkResult;
  ImGui_ImplVulkan_Init(&init_info, **renderPass);

  uploadFonts();

  swapChain->addRebuildListener(
      [&] { ImGui_ImplVulkan_SetMinImageCount(swapChain->getImageViews().size()); });
}

void ImGuiGlfwVulkan::setupDescriptorPool() {
  constexpr auto DESCRIPTOR_COUNT = 1000;
  auto descPoolConfig = vulkan::DescriptorPoolConfig();
  descPoolConfig.poolSizes = {{vk::DescriptorType::eSampler, DESCRIPTOR_COUNT},
                              {vk::DescriptorType::eCombinedImageSampler, DESCRIPTOR_COUNT},
                              {vk::DescriptorType::eSampledImage, DESCRIPTOR_COUNT},
                              {vk::DescriptorType::eStorageImage, DESCRIPTOR_COUNT},
                              {vk::DescriptorType::eUniformTexelBuffer, DESCRIPTOR_COUNT},
                              {vk::DescriptorType::eStorageTexelBuffer, DESCRIPTOR_COUNT},
                              {vk::DescriptorType::eUniformBuffer, DESCRIPTOR_COUNT},
                              {vk::DescriptorType::eStorageBuffer, DESCRIPTOR_COUNT},
                              {vk::DescriptorType::eUniformBufferDynamic, DESCRIPTOR_COUNT},
                              {vk::DescriptorType::eStorageBufferDynamic, DESCRIPTOR_COUNT},
                              {vk::DescriptorType::eInputAttachment, DESCRIPTOR_COUNT}};
  descPoolConfig.maxSets = DESCRIPTOR_COUNT * descPoolConfig.poolSizes.size();
  descPoolConfig.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
  descriptorPool = logicalDevice->createDescriptorPool(std::move(descPoolConfig));
}

void ImGuiGlfwVulkan::uploadFonts() {
  auto commandPoolConfig = vulkan::CommandPoolConfig();
  commandPoolConfig.queueFamily = vk::QueueFlagBits::eGraphics;
  auto commandPool = logicalDevice->createCommandPool(std::move(commandPoolConfig));

  auto commandBufferConfig = vulkan::CommandBufferConfig();
  commandBufferConfig.count = 1;
  commandBufferConfig.level = vk::CommandBufferLevel::ePrimary;
  auto commandBuffer = commandPool->createCommandBuffers(std::move(commandBufferConfig))[0];

  auto recorder = commandBuffer->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  ImGui_ImplVulkan_CreateFontsTexture(**commandBuffer);
  recorder.end();

  auto fence = logicalDevice->createFence({.flags = {}});

  commandPool->submitCommandBuffers({.commandBuffers = {*commandBuffer},
                                     .waitSemaphores = {},
                                     .signalSemaphores = {},
                                     .flags = {},
                                     .fence = *fence,
                                     .wait = true});

  logicalDevice->wait();

  ImGui_ImplVulkan_DestroyFontUploadObjects();
}
void ImGuiGlfwVulkan::render() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  std::ranges::for_each(getChildren(), [] (auto &child) {
    child.second->render();
  });
  ImGui::Render();
}

ImGuiGlfwVulkan::~ImGuiGlfwVulkan() {
  logicalDevice->wait();
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void ImGuiGlfwVulkan::addToCommandBuffer(vulkan::CommandBufferRecording &recording) {
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *recording.getCommandBuffer());
}

}// namespace pf::ui