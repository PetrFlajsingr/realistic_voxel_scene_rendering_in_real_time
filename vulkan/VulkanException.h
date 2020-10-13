//
// Created by petr on 9/25/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKANEXCEPTION_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKANEXCEPTION_H

#include "../exceptions/StackTraceException.h"

class VulkanException : public StackTraceException {
 public:
  explicit VulkanException(const std::string_view &message);
  static VulkanException fmt(std::string_view fmt, auto &&... args) {
    return VulkanException(fmt::format(fmt, args...));
  }
};

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKANEXCEPTION_H
