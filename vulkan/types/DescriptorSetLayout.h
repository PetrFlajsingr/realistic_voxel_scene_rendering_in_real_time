//
// Created by petr on 9/28/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_DESCRIPTORSETLAYOUT_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_DESCRIPTORSETLAYOUT_H

#include "../concepts/PtrConstructable.h"
#include "fwd.h"
#include "VulkanObject.h"
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
  LogicalDevice &logicalDevice;
};

class DescriptorSetLayout : public VulkanObject, public PtrConstructable<DescriptorSetLayout> {
 public:
  explicit DescriptorSetLayout(const DescriptorSetLayoutConfig &config);

  DescriptorSetLayout(const DescriptorSetLayout &other) = delete;
  DescriptorSetLayout &operator=(const DescriptorSetLayout &other) = delete;

  [[nodiscard]] const vk::DescriptorSetLayout &getLayout() const;

  [[nodiscard]] std::string info() const override;

  const vk::DescriptorSetLayout &operator*() const;
  vk::DescriptorSetLayout const *operator->() const;

 private:
  vk::UniqueDescriptorSetLayout vkSet;
};

}// namespace pf::vulkan
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_DESCRIPTORSETLAYOUT_H
