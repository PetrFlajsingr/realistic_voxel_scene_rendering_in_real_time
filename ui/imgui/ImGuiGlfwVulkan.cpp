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
                                 std::shared_ptr<vulkan::Surface> surf,
                                 std::shared_ptr<vulkan::SwapChain> swapCh, GLFWwindow *handle,
                                 ImGuiConfigFlags flags)
    : ImGuiBase(flags), logicalDevice(std::move(device)), surface(std::move(surf)),
      swapChain(std::move(swapCh))  {

  auto &physicalDevice = logicalDevice->getPhysicalDevice();
  const auto imageCount = swapChain->getImageViews().size();

  setupDescriptorPool();
  setupRenderPass();
  ImGui_ImplGlfw_InitForVulkan(handle, true);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = *physicalDevice.getInstance();
  init_info.PhysicalDevice = *physicalDevice;
  init_info.Device = **logicalDevice;
  init_info.QueueFamily = logicalDevice->getQueueIndices()[vk::QueueFlagBits::eGraphics];
  init_info.Queue = logicalDevice->getQueue(vk::QueueFlagBits::eGraphics);
  init_info.PipelineCache = VK_NULL_HANDLE;
  init_info.DescriptorPool = **descriptorPool;
  init_info.Allocator = nullptr;
  init_info.MinImageCount = imageCount;
  init_info.ImageCount = imageCount;
  init_info.CheckVkResultFn = details::checkVkResult;
  ImGui_ImplVulkan_Init(&init_info, **renderPass);

  uploadFonts();

  swapChain->addRebuildListener([&] {
    ImGui_ImplVulkan_SetMinImageCount(swapChain->getImageViews().size());
  });
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

void ImGuiGlfwVulkan::setupRenderPass() {
  // clang-format off
  renderPass = vulkan::RenderPassBuilder(logicalDevice)
                  .attachment("a1")
                    .format(swapChain->getFormat())
                    .samples(vk::SampleCountFlagBits::e1)
                    .loadOp(vk::AttachmentLoadOp::eLoad)
                    .storeOp(vk::AttachmentStoreOp::eStore)
                    .stencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                    .stencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                    .initialLayout(vk::ImageLayout::eColorAttachmentOptimal)
                    .finalLayout(vk::ImageLayout::ePresentSrcKHR)
                  .attachmentDone()
                  .subpass("s1")
                    .colorAttachment("a1")
                    .dependency()
                      .srcSubpass()
                      .dstSubpass("s1")
                      .srcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                      .dstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                      .srcAccessFlags(vk::AccessFlagBits::eColorAttachmentWrite)
                      .dstAccessFlags(vk::AccessFlagBits::eColorAttachmentWrite)
                    .dependencyDone()
                  .subpassDone()
                  .build();
  // clang-format on
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


  // TODO
  /*auto commandSubmitConfig = vulkan::MultiCommandSubmitConfig();
  commandSubmitConfig.commandBuffers = {*commandBuffer};
  commandSubmitConfig.wait = true;
  commandPool->submitCommandBuffers(commandSubmitConfig);*/

  ImGui_ImplVulkan_DestroyFontUploadObjects();
}
void ImGuiGlfwVulkan::render() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::ShowTestWindow();
  ImGui::Render();
}

}// namespace pf::ui