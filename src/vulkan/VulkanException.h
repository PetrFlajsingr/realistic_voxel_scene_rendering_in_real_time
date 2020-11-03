//
// Created by petr on 9/25/20.
//

#ifndef VOXEL_RENDER_VULKANEXCEPTION_H
#define VOXEL_RENDER_VULKANEXCEPTION_H

#include "exceptions/StackTraceException.h"

namespace pf::vulkan {
class VulkanException : public StackTraceException {
 public:
  explicit VulkanException(const std::string_view &message);
  static VulkanException fmt(std::string_view fmt, auto &&... args) {
    return VulkanException(fmt::format(fmt, args...));
  }
};
}// namespace pf::vulkan
#endif//VOXEL_RENDER_VULKANEXCEPTION_H
