//
// Created by petr on 10/18/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_COMMANDBUFFER_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_COMMANDBUFFER_H

#include "../concepts/PtrConstructible.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

struct CommandBufferConfig {
  vk::CommandBufferLevel level;
  uint32_t count;
};

class CommandBuffer : public VulkanObject, public PtrConstructible<CommandBuffer> {
 public:
  explicit CommandBuffer(std::shared_ptr<CommandPool> pool, vk::UniqueCommandBuffer &&buffer);

  CommandBuffer(const CommandBuffer &other) = delete;
  CommandBuffer &operator=(const CommandBuffer &other) = delete;

  void begin(vk::CommandBufferUsageFlagBits flag);
  void end();
  void reset();

  [[nodiscard]] CommandPool &getCommandPool();

  [[nodiscard]] const vk::CommandBuffer &getVkBuffer() const;

  const vk::CommandBuffer &operator*() const;
  vk::CommandBuffer const *operator->() const;

  [[nodiscard]] std::string info() const override;

 private:
  vk::UniqueCommandBuffer vkBuffer;
  std::shared_ptr<CommandPool> commandPool;
};
}

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_COMMANDBUFFER_H
