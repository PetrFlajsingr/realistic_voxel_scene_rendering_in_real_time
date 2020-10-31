//
// Created by petr on 10/27/20.
//

#ifndef VOXEL_RENDER_DESCRIPTORPOOL_H
#define VOXEL_RENDER_DESCRIPTORPOOL_H

#include "../concepts/PtrConstructible.h"
#include "fwd.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

struct DescriptorPoolConfig {
  vk::DescriptorPoolCreateFlags flags;
  uint32_t maxSets;
  std::vector<vk::DescriptorPoolSize> poolSizes;
};

class DescriptorPool : public PtrConstructible<DescriptorPool> {
 public:
  DescriptorPool(std::shared_ptr<LogicalDevice> device, DescriptorPoolConfig &&config);

  [[nodiscard]] vk::DescriptorPool &getDescriptorPool();
  vk::DescriptorPool &operator*();
  vk::DescriptorPool *operator->();

  [[nodiscard]] LogicalDevice &getDevice();

 private:
  vk::UniqueDescriptorPool vkDescriptorPool;
  std::shared_ptr<LogicalDevice> logicalDevice;
};
}// namespace pf::vulkan
#endif//VOXEL_RENDER_DESCRIPTORPOOL_H
