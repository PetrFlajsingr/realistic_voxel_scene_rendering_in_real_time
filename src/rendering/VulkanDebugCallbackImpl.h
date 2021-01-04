//
// Created by petr on 12/5/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_RENDERING_VULKANDEBUGCALLBACKIMPL_H
#define REALISTIC_VOXEL_RENDERING_SRC_RENDERING_VULKANDEBUGCALLBACKIMPL_H

#include <pf_glfw_vulkan/vulkan/types/VulkanCommon.h>

namespace pf {
class VulkanDebugCallbackImpl {
 public:
  static bool debugCallback(const vulkan::DebugCallbackData &data, vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                            const vk::DebugUtilsMessageTypeFlagsEXT &);

 private:
  static std::string makeValidationMessageReadable(std::string message, uint maxLineLength);
};
}// namespace pf
#endif//REALISTIC_VOXEL_RENDERING_SRC_RENDERING_VULKANDEBUGCALLBACKIMPL_H
