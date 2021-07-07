/**
 * @file GBufferRenderer.h
 * @brief A renderer using ESVO to render gbuffer.
 * @author Petr Flajšingr
 * @date 30.6.21
 */

#ifndef REALISTIC_VOXEL_RENDERING_SRC_RENDERING_GBUFFERRENDERER_H
#define REALISTIC_VOXEL_RENDERING_SRC_RENDERING_GBUFFERRENDERER_H

#include "enums.h"
#include <filesystem>
#include <memory>
#include <pf_glfw_vulkan/vulkan/types/ComputePipeline.h>
#include <pf_glfw_vulkan/vulkan/types/fwd.h>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace pf {
/**
 * @brief G-buffer renderer.
 *
 * Uses an ESVO ray tracing algorithm to save hit positions, normals and material info into a g buffer.
 */
class GBufferRenderer {
 public:
  GBufferRenderer(std::filesystem::path shaderDir, vk::Extent2D viewportSize,
                  std::shared_ptr<vulkan::LogicalDevice> vkLogicalDevice,
                  const std::shared_ptr<vulkan::CommandPool> &vkCommandPool, std::shared_ptr<vulkan::Buffer> bufferSVO,
                  std::shared_ptr<vulkan::Buffer> bufferModelInfo, std::shared_ptr<vulkan::Buffer> bufferBVH,
                  std::shared_ptr<vulkan::Buffer> bufferLight, std::shared_ptr<vulkan::Buffer> bufferCamera,
                  std::shared_ptr<vulkan::Buffer> bufferMaterials, vk::Format presentFormat);

  std::shared_ptr<vulkan::Semaphore> render();

  [[nodiscard]] const std::shared_ptr<vulkan::Image> &getPosAndMaterialImage() const;
  [[nodiscard]] const std::shared_ptr<vulkan::ImageView> &getPosAndMaterialImageView() const;
  [[nodiscard]] const std::shared_ptr<vulkan::Image> &getNormalImage() const;
  [[nodiscard]] const std::shared_ptr<vulkan::ImageView> &getNormalImageView() const;

  [[nodiscard]] const std::shared_ptr<vulkan::Image> &getDebugImage() const;
  [[nodiscard]] const std::shared_ptr<vulkan::ImageView> &getDebugImageView() const;
  [[nodiscard]] const std::shared_ptr<vulkan::TextureSampler> &getDebugImageSampler() const;

  void setViewType(GBufferViewType viewType);

 private:
  void createTextures(vk::Format presentFormat);

  void createDescriptorPools();

  void createPipeline();

  void createCommands(vulkan::CommandPool &pool);

  void recordCommands();

  void createFences();
  void createSemaphores();

  std::shared_ptr<vulkan::LogicalDevice> logicalDevice;
  vk::Extent2D extent2D;
  std::filesystem::path shaderPath;
  std::shared_ptr<vulkan::Buffer> svoBuffer;
  std::shared_ptr<vulkan::Buffer> modelInfoBuffer;
  std::shared_ptr<vulkan::Buffer> bvhBuffer;
  std::shared_ptr<vulkan::Buffer> lightUniformBuffer;
  std::shared_ptr<vulkan::Buffer> cameraUniformBuffer;
  std::shared_ptr<vulkan::Buffer> materialsBuffer;

  std::shared_ptr<vulkan::Buffer> debugUniformBuffer;

  std::shared_ptr<vulkan::Image> posAndMaterialImage;
  std::shared_ptr<vulkan::ImageView> posAndMaterialImageView;
  std::shared_ptr<vulkan::Image> normalImage;
  std::shared_ptr<vulkan::ImageView> normalImageView;

  std::shared_ptr<vulkan::Image> debugImage;
  std::shared_ptr<vulkan::ImageView> debugImageView;
  std::shared_ptr<vulkan::TextureSampler> debugImageSampler;

  std::shared_ptr<vulkan::DescriptorPool> descriptorPool;
  std::vector<vk::UniqueDescriptorSet> descriptorSets;
  std::shared_ptr<vulkan::DescriptorSetLayout> descriptorSetLayout;
  std::shared_ptr<vulkan::ComputePipeline> computePipeline;

  std::shared_ptr<vulkan::Fence> fence;
  std::shared_ptr<vulkan::Semaphore> semaphore;

  std::shared_ptr<vulkan::CommandBuffer> commandBuffer;
};
}// namespace pf
#endif//REALISTIC_VOXEL_RENDERING_SRC_RENDERING_GBUFFERRENDERER_H
