//
// Created by petr on 6/19/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_RENDERING_LIGHT_FIELD_PROBES_PROBERENDERER_H
#define REALISTIC_VOXEL_RENDERING_SRC_RENDERING_LIGHT_FIELD_PROBES_PROBERENDERER_H

#include "ProbeManager.h"
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
#include "enums.h"
namespace pf::lfp {

class ProbeRenderer {
 public:

  ProbeRenderer(toml::table config, std::shared_ptr<vulkan::Instance> vkInstance,
                std::shared_ptr<vulkan::PhysicalDevice> vkDevice,
                std::shared_ptr<vulkan::LogicalDevice> vkLogicalDevice, std::shared_ptr<vulkan::Buffer> svoBuffer,
                std::shared_ptr<vulkan::Buffer> modelInfoBuffer, std::shared_ptr<vulkan::Buffer> bvhBuffer,
                std::unique_ptr<ProbeManager> probeManag);

  [[nodiscard]] const std::shared_ptr<vulkan::Image> &getProbesDebugImage() const;
  [[nodiscard]] const std::shared_ptr<vulkan::ImageView> &getProbesDebugImageView() const;
  [[nodiscard]] const std::shared_ptr<vulkan::TextureSampler> &getProbesDebugSampler() const;
  [[nodiscard]] const std::shared_ptr<vulkan::Buffer> &getProbeGenDebugUniformBuffer() const;
  [[nodiscard]] const std::shared_ptr<vulkan::Buffer> &getRenderDebugUniformBuffer() const;

  const std::shared_ptr<vulkan::Semaphore> &renderProbeTextures();

  const std::shared_ptr<vulkan::Semaphore> &render();

  [[nodiscard]] ProbeManager &getProbeManager();

  void setProbeToRender(std::uint32_t index);
  void setProbeToRender(glm::ivec3 position);

  void setProbeDebugRenderType(ProbeVisualisation type);

  void renderProbesInNextPass();

 private:
  bool renderingProbesInNextPass = false;
  toml::table config;
  std::shared_ptr<vulkan::Instance> vkInstance;
  std::shared_ptr<vulkan::PhysicalDevice> vkDevice;
  std::shared_ptr<vulkan::LogicalDevice> vkLogicalDevice;

  struct {
    std::shared_ptr<vulkan::DescriptorPool> vkDescPool;
    std::shared_ptr<vulkan::DescriptorSetLayout> vkComputeDescSetLayout;
    std::vector<vk::UniqueDescriptorSet> computeDescriptorSets;

    std::shared_ptr<vulkan::CommandPool> vkCommandPool;
    std::shared_ptr<vulkan::ComputePipeline> vkComputePipeline;
    std::shared_ptr<vulkan::Semaphore> vkComputeSemaphore;
    std::shared_ptr<vulkan::CommandBuffer> vkCommandBuffer;
    std::shared_ptr<vulkan::Buffer> debugUniformBuffer;
  } probeGenData;


  std::shared_ptr<vulkan::Buffer> svoBuffer;
  std::shared_ptr<vulkan::Buffer> modelInfoBuffer;
  std::shared_ptr<vulkan::Buffer> bvhBuffer;
  std::shared_ptr<vulkan::Buffer> gridInfoBuffer;

  struct {
    std::shared_ptr<vulkan::DescriptorPool> vkDescPool;
    std::shared_ptr<vulkan::DescriptorSetLayout> vkComputeDescSetLayout;
    std::vector<vk::UniqueDescriptorSet> computeDescriptorSets;

    std::shared_ptr<vulkan::CommandPool> vkCommandPool;
    std::shared_ptr<vulkan::ComputePipeline> vkComputePipeline;
    std::shared_ptr<vulkan::Semaphore> vkComputeSemaphore;
    std::shared_ptr<vulkan::CommandBuffer> vkCommandBuffer;
    std::shared_ptr<vulkan::Buffer> debugUniformBuffer;
  }renderData;

 public:
  std::unique_ptr<ProbeManager> probeManager;

 private:

  void createProbeGenDescriptorPool();
  void createProbeGenPipeline();
  void createProbeGenCommands();
  void recordProbeGenCommands();


  void createRenderDescriptorPool();
  void createRenderPipeline();
  void createRenderCommands();
  void recordRenderCommands();

  void createFences();
  void createTextures();

  std::shared_ptr<vulkan::Fence> vkComputeFence;
  std::shared_ptr<vulkan::Image> vkProbesDebugImage;

  std::shared_ptr<vulkan::ImageView> vkProbesDebugImageView;


  std::shared_ptr<vulkan::TextureSampler> vkProbesDebugImageSampler;
};
}// namespace pf::lfp
#endif//REALISTIC_VOXEL_RENDERING_SRC_RENDERING_LIGHT_FIELD_PROBES_PROBERENDERER_H
