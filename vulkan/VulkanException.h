//
// Created by petr on 9/25/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKANEXCEPTION_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKANEXCEPTION_H

#include "../exceptions/StackTraceException.h"

namespace pf::vulkan {
class VulkanException : public StackTraceException {
 public:
  explicit VulkanException(const std::string_view &message);
  static VulkanException fmt(std::string_view fmt, auto &&... args) {
    return VulkanException(fmt::format(fmt, args...));
  }
};
}// namespace pf::vulkan
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKANEXCEPTION_H
