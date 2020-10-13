//
// Created by petr on 9/26/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SWAPCHAIN_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SWAPCHAIN_H

#include "Device.h"
#include "ImageView.h"
#include "VulkanObject.h"
#include <set>
#include <vector>
#include <unordered_set>
#include <vulkan/vulkan.hpp>
#include "../concepts/PtrConstructable.h"

namespace pf::vulkan {

struct SwapChainConfig {
  std::set<vk::SurfaceFormatKHR> formats;
  std::set<vk::PresentModeKHR> presentModes;
  std::pair<uint32_t, uint32_t> resolution;
  vk::ImageUsageFlags imageUsage;
  std::unordered_set<uint32_t> sharingQueues;
  uint32_t imageArrayLayers;
  bool clipped;
  std::optional<vk::SwapchainKHR> oldSwapChain;
  vk::CompositeAlphaFlagBitsKHR compositeAlpha;

  Device &device;
  Surface &surface;
  LogicalDevice &logicalDevice;
};

class SwapChain : public VulkanObject, public PtrConstructable<SwapChain> {
 public:
  explicit SwapChain(const pf::vulkan::SwapChainConfig &config);

  SwapChain(const SwapChain&) = delete;
  SwapChain &operator=(const SwapChain&) = delete;

  [[nodiscard]] std::vector<std::shared_ptr<ImageView>> createImageViews(LogicalDevice &dev);
  [[nodiscard]] vk::Format getFormat() const;
  [[nodiscard]] vk::ColorSpaceKHR getColorSpace() const;
  [[nodiscard]] const vk::Extent2D &getExtent() const;

  const vk::SwapchainKHR &operator*() const;
  vk::SwapchainKHR const * operator->() const;

  [[nodiscard]] std::string info() const override;

 private:

  static vk::PresentModeKHR selectPresentMode(
      const std::set<vk::PresentModeKHR> &present_modes,
      const std::vector<vk::PresentModeKHR> &surface_present_modes);

  static vk::SurfaceFormatKHR selectSurfaceFormat(
      const std::set<vk::SurfaceFormatKHR> &formats,
      const std::vector<vk::SurfaceFormatKHR> &surfaceFormats);

  static vk::Extent2D selectExtent(const std::pair<uint32_t, uint32_t> &resolution,
                                     const vk::SurfaceCapabilitiesKHR &surfaceCapabilities);

  static vk::UniqueSwapchainKHR createSwapChainHandle(
      const vk::SurfaceCapabilitiesKHR &surfaceCapabilities,
      const pf::vulkan::SwapChainConfig &config, vk::SurfaceFormatKHR surfaceFormat,
      vk::Extent2D extent, vk::PresentModeKHR presentMode);

  vk::UniqueSwapchainKHR vkSwapChain;
  vk::Format format;
  vk::ColorSpaceKHR colorSpace;
  vk::Extent2D extent;
};
}// namespace pf::vulkan
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SWAPCHAIN_H
