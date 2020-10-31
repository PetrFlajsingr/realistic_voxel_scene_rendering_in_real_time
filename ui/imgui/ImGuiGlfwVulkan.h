//
// Created by petr on 10/27/20.
//

#ifndef VOXEL_RENDER_IMGUIGLFWVULKAN_H
#define VOXEL_RENDER_IMGUIGLFWVULKAN_H

#include "../vulkan/types/fwd.h"
#include "ImGuiInterface.h"
#include <GLFW/glfw3.h>
#include <imgui/imgui_impl_vulkan.h>
#include <memory>
#include <vulkan/vulkan.hpp>

namespace pf::ui {

class ImGuiGlfwVulkan : public ImGuiInterface {
 public:
  ImGuiGlfwVulkan(std::shared_ptr<vulkan::LogicalDevice> device, std::shared_ptr<vulkan::RenderPass> pass,
                  std::shared_ptr<vulkan::Surface> surf, std::shared_ptr<vulkan::SwapChain> swapCh,
                  GLFWwindow *handle, ImGuiConfigFlags flags);

  void addToCommandBuffer(vulkan::CommandBufferRecording &recording) override;

  ~ImGuiGlfwVulkan() override;

  void render() override;

 private:
  void setupDescriptorPool();
  void uploadFonts();
  std::shared_ptr<vulkan::DescriptorPool> descriptorPool;
  std::shared_ptr<vulkan::LogicalDevice> logicalDevice;
  std::shared_ptr<vulkan::RenderPass> renderPass;
  std::shared_ptr<vulkan::Surface> surface;
  std::shared_ptr<vulkan::SwapChain> swapChain;
};

}// namespace pf::ui
#endif//VOXEL_RENDER_IMGUIGLFWVULKAN_H
