//
// Created by petr on 9/28/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_COMMANDPOOL_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_COMMANDPOOL_H

#include "VulkanObject.h"
#include "../concepts/PtrConstructable.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {
class CommandPool : public VulkanObject, public PtrConstructable<CommandPool> {
 public:
  // TODO: build from config
  explicit CommandPool(vk::UniqueCommandPool &&vkCommandPool);

  CommandPool(const CommandPool &other) = delete;
  CommandPool &operator=(const CommandPool &other) = delete;

  [[nodiscard]] const vk::CommandPool &getCommandPool() const;

  const vk::CommandPool &operator*() const;
  vk::CommandPool const * operator->() const;

  [[nodiscard]] std::string info() const override;

 private:
  vk::UniqueCommandPool vkCommandPool;
};

}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_COMMANDPOOL_H
