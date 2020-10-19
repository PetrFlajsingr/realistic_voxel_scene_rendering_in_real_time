//
// Created by petr on 10/18/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_LOGICALDEVICE_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_LOGICALDEVICE_H

#include "../concepts/OneOf.h"
#include "../concepts/PtrConstructible.h"
#include "../concepts/Window.h"
#include "CommandPool.h"
#include "DescriptorSetLayout.h"
#include "Image.h"
#include "Shader.h"
#include "SwapChain.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <range/v3/action.hpp>
#include <range/v3/view.hpp>
#include <string>
#include <unordered_set>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {
using LogicalDeviceId = std::string;

struct LogicalDeviceConfig {
  LogicalDeviceId id;
  vk::PhysicalDeviceFeatures deviceFeatures;
  std::unordered_set<vk::QueueFlagBits> queueTypes;
  bool presentQueueEnabled{};
  std::unordered_set<std::string> requiredDeviceExtensions;
  std::unordered_set<std::string> validationLayers;
  Surface &surface;
};

class LogicalDevice : public VulkanObject,
                      public PtrConstructible<LogicalDevice>,
                      public std::enable_shared_from_this<LogicalDevice> {
 public:
  LogicalDevice(const std::shared_ptr<PhysicalDevice> &device, vk::UniqueDevice &&vkLogicalDevice,
                std::unordered_map<vk::QueueFlagBits, uint32_t> queueIndices,
                const std::optional<uint32_t> &presentQueueIndex);

  LogicalDevice(const LogicalDevice &other) = delete;
  LogicalDevice &operator=(const LogicalDevice &other) = delete;

  [[nodiscard]] const vk::Device &getVkLogicalDevice() const;
  [[nodiscard]] std::unordered_map<vk::QueueFlagBits, uint32_t> &getQueueIndices();
  [[nodiscard]] vk::Queue getQueue(vk::QueueFlagBits type);
  [[nodiscard]] const std::optional<uint32_t> &getPresentQueueIndex() const;

  const vk::Device &operator*() const;
  vk::Device const *operator->() const;

  [[nodiscard]] std::shared_ptr<ImageUnique> createImage(ImageConfig &&config);
  [[nodiscard]] std::shared_ptr<SwapChain> createSwapChain(std::shared_ptr<Surface> surface,
                                                           SwapChainConfig &&config);
  [[nodiscard]] std::shared_ptr<CommandPool> createCommandPool(CommandPoolConfig &&config);
  [[nodiscard]] std::shared_ptr<DescriptorSetLayout>
  createDescriptorSetLayout(DescriptorSetLayoutConfig &&config);
  template <OneOf<ShaderConfigFile, ShaderConfigSrc, ShaderConfigGlslSrc, ShaderConfigGlslFile> T>
  [[nodiscard]] std::shared_ptr<Shader> createShader(T &&config) {
    return Shader::CreateShared(shared_from_this(), std::move(config));
  }

  [[nodiscard]] PhysicalDevice &getPhysicalDevice() const;

  void wait();

  std::string info() const override;

 private:
  std::weak_ptr<PhysicalDevice> physDevice;
  vk::UniqueDevice vkLogicalDevice;
  std::unordered_map<vk::QueueFlagBits, uint32_t> queueIndices;
  std::optional<uint32_t> presentQueueIndex;
};
}// namespace pf::vulkan
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_LOGICALDEVICE_H
