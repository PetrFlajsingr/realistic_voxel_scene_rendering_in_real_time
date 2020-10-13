//
// Created by petr on 9/26/20.
//

#include "TriangleRenderer.h"

bool pf::TriangleRenderer::debugCallback(const pf::vulkan::DebugCallbackData &data,
                                           vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                           const vk::DebugUtilsMessageTypeFlagsEXT &) {
  auto log_level = spdlog::level::trace;
  switch (severity) {
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
      log_level = spdlog::level::trace;
      break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo: log_level = spdlog::level::info; break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning: log_level = spdlog::level::warn; break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError: log_level = spdlog::level::err; break;
  }
  logFmt(log_level, VK_TAG, "Validation layer: {} message id: {}", data.message, data.messageId);
  return false;
}

vk::Format pf::TriangleRenderer::getDepthFormat() {
  const auto tiling = vk::ImageTiling::eLinear;
  const auto features = vk::FormatFeatureFlagBits::eDepthStencilAttachment;
  return vk::Format::eD32Sfloat;
  for (auto format :
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}) {
    const auto properties = vkDevice->getPhysicalDevice().getFormatProperties(format);

    if (tiling == vk::ImageTiling::eLinear
        && (properties.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == vk::ImageTiling::eOptimal
        && (properties.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}
