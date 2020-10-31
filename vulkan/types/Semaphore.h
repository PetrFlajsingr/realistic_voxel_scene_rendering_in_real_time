//
// Created by petr on 10/28/20.
//

#ifndef VOXEL_RENDER_SEMAPHORE_H
#define VOXEL_RENDER_SEMAPHORE_H

#include "../concepts/PtrConstructible.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

class Semaphore : public VulkanObject, public PtrConstructible<Semaphore> {
 public:
  explicit Semaphore(std::shared_ptr<LogicalDevice> device);

  const vk::Semaphore &operator*() const;
  vk::Semaphore const *operator->() const;

  [[nodiscard]] const vk::Semaphore &getVkSemaphore() const;

  [[nodiscard]] LogicalDevice &getLogicalDevice() const;

  [[nodiscard]] std::string info() const override;

 private:
  std::shared_ptr<LogicalDevice> logicalDevice;
  vk::UniqueSemaphore vkSemaphore;
};

}// namespace pf::vulkan

#endif//VOXEL_RENDER_SEMAPHORE_H
