/**
 * @file VulkanDebugCallbackImpl.h
 * @brief Implementation for vulkan debug callback.
 * @author Petr Flajšingr
 * @date 5.12.20
 */

#ifndef REALISTIC_VOXEL_RENDERING_SRC_RENDERING_VULKANDEBUGCALLBACKIMPL_H
#define REALISTIC_VOXEL_RENDERING_SRC_RENDERING_VULKANDEBUGCALLBACKIMPL_H

#include <pf_glfw_vulkan/vulkan/types/VulkanCommon.h>

namespace pf {
/**
 * @brief A class which provides a debug callback function for Vulkan validation layers.
 */
class VulkanDebugCallbackImpl {
 public:
  static bool debugCallback(const vulkan::DebugCallbackData &data, vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                            const vk::DebugUtilsMessageTypeFlagsEXT &);

 private:
  static std::string makeValidationMessageReadable(std::string message, uint32_t maxLineLength);
};
}// namespace pf
#endif//REALISTIC_VOXEL_RENDERING_SRC_RENDERING_VULKANDEBUGCALLBACKIMPL_H
