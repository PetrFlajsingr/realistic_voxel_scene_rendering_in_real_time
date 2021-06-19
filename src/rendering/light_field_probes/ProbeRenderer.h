//
// Created by petr on 6/19/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_RENDERING_LIGHT_FIELD_PROBES_PROBERENDERER_H
#define REALISTIC_VOXEL_RENDERING_SRC_RENDERING_LIGHT_FIELD_PROBES_PROBERENDERER_H

#include <memory>
#include <pf_common/ByteLiterals.h>
#include <pf_glfw_vulkan/vulkan/types/Buffer.h>
#include <pf_glfw_vulkan/vulkan/types/CommandBuffer.h>
#include <pf_glfw_vulkan/vulkan/types/CommandPool.h>
#include <pf_glfw_vulkan/vulkan/types/ComputePipeline.h>
#include <pf_glfw_vulkan/vulkan/types/DescriptorPool.h>
#include <pf_glfw_vulkan/vulkan/types/DescriptorSetLayout.h>
#include <pf_glfw_vulkan/vulkan/types/Fence.h>
#include <pf_glfw_vulkan/vulkan/types/Image.h>
#include <pf_glfw_vulkan/vulkan/types/ImageView.h>
#include <pf_glfw_vulkan/vulkan/types/Instance.h>
#include <pf_glfw_vulkan/vulkan/types/LogicalDevice.h>
#include <pf_glfw_vulkan/vulkan/types/PhysicalDevice.h>
#include <pf_glfw_vulkan/vulkan/types/Semaphore.h>
#include <pf_glfw_vulkan/vulkan/types/Shader.h>
#include <pf_glfw_vulkan/vulkan/types/TextureSampler.h>
#include <toml++/toml.h>

namespace pf::lfp {
class ProbeRenderer {
 public:
  constexpr static auto PROBES_IMG_ARRAY_LAYERS = 50;

  ProbeRenderer(toml::table config, std::shared_ptr<vulkan::Instance> vkInstance,
                std::shared_ptr<vulkan::PhysicalDevice> vkDevice,
                std::shared_ptr<vulkan::LogicalDevice> vkLogicalDevice, std::shared_ptr<vulkan::Buffer> svoBuffer,
                std::shared_ptr<vulkan::Buffer> modelInfoBuffer, std::shared_ptr<vulkan::Buffer> bvhBuffer);

  [[nodiscard]] const std::shared_ptr<vulkan::Image> &getProbesImage() const;
  [[nodiscard]] const std::shared_ptr<vulkan::ImageView> &getProbesImageView() const;
  [[nodiscard]] const std::shared_ptr<vulkan::Image> &getProbesDebugImage() const;
  [[nodiscard]] const std::shared_ptr<vulkan::ImageView> &getProbesDebugImageView() const;
  [[nodiscard]] const std::shared_ptr<vulkan::TextureSampler> &getProbesDebugSampler() const;
  [[nodiscard]] const std::shared_ptr<vulkan::Buffer> &getProbePosBuffer() const;
  [[nodiscard]] const std::shared_ptr<vulkan::Buffer> &getDebugUniformBuffer() const;

  const std::shared_ptr<vulkan::Semaphore> &render();

 private:
  void recordCommands();
  toml::table config;
  std::shared_ptr<vulkan::Instance> vkInstance;
  std::shared_ptr<vulkan::PhysicalDevice> vkDevice;
  std::shared_ptr<vulkan::LogicalDevice> vkLogicalDevice;
  std::shared_ptr<vulkan::DescriptorPool> vkDescPool;
  std::shared_ptr<vulkan::DescriptorSetLayout> vkComputeDescSetLayout;
  std::vector<vk::UniqueDescriptorSet> computeDescriptorSets;

  std::shared_ptr<vulkan::Buffer> svoBuffer;
  std::shared_ptr<vulkan::Buffer> modelInfoBuffer;
  std::shared_ptr<vulkan::Buffer> bvhBuffer;
  std::shared_ptr<vulkan::Buffer> debugUniformBuffer;
  std::shared_ptr<vulkan::Buffer> probePosBuffer;

  std::shared_ptr<vulkan::CommandPool> vkCommandPool;

  std::shared_ptr<vulkan::ComputePipeline> vkComputePipeline;
  std::shared_ptr<vulkan::Fence> vkComputeFence;
  std::shared_ptr<vulkan::Semaphore> vkComputeSemaphore;

  std::shared_ptr<vulkan::CommandBuffer> vkCommandBuffer;

  void createDescriptorPool();

  void createTextures();

  std::shared_ptr<vulkan::Image> vkProbesImage;
  std::shared_ptr<vulkan::ImageView> vkProbesImageView;
  std::shared_ptr<vulkan::Image> vkProbesDebugImage;
  std::shared_ptr<vulkan::ImageView> vkProbesDebugImageView;
  std::shared_ptr<vulkan::TextureSampler> vkProbesDebugImageSampler;

  void createPipeline();

  void createCommands();

  void createFences();
};
}// namespace pf::lfp
#endif//REALISTIC_VOXEL_RENDERING_SRC_RENDERING_LIGHT_FIELD_PROBES_PROBERENDERER_H
