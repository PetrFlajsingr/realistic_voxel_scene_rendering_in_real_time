//
// Created by petr on 9/28/20.
//

#ifndef VOXEL_RENDER_DESCRIPTORSETLAYOUT_H
#define VOXEL_RENDER_DESCRIPTORSETLAYOUT_H

#include "concepts/PtrConstructible.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

struct DescriptorSetLayoutBindingConfig {
  uint32_t binding;
  vk::DescriptorType type;
  uint32_t count;
  vk::ShaderStageFlags stageFlags;
};

struct DescriptorSetLayoutConfig {
  std::vector<DescriptorSetLayoutBindingConfig> bindings;
};

class DescriptorSetLayout : public VulkanObject, public PtrConstructible<DescriptorSetLayout> {
 public:
  explicit DescriptorSetLayout(std::shared_ptr<LogicalDevice> device, DescriptorSetLayoutConfig &&config);

  DescriptorSetLayout(const DescriptorSetLayout &other) = delete;
  DescriptorSetLayout &operator=(const DescriptorSetLayout &other) = delete;

  [[nodiscard]] const vk::DescriptorSetLayout &getLayout() const;
  [[nodiscard]] LogicalDevice &getDevice();
  [[nodiscard]] std::string info() const override;

  const vk::DescriptorSetLayout &operator*() const;
  vk::DescriptorSetLayout const *operator->() const;

 private:
  std::shared_ptr<LogicalDevice> logicalDevice;
  vk::UniqueDescriptorSetLayout vkSet;
};

}// namespace pf::vulkan
#endif//VOXEL_RENDER_DESCRIPTORSETLAYOUT_H
