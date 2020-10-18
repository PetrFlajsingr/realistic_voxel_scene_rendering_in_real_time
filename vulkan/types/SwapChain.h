//
// Created by petr on 9/26/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SWAPCHAIN_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SWAPCHAIN_H

#include "../concepts/PtrConstructable.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <set>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan.hpp>

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
};

class SwapChain : public VulkanObject, public PtrConstructable<SwapChain> {
 public:
  explicit SwapChain(std::shared_ptr<Surface> surf, std::shared_ptr<LogicalDevice> device,
                     pf::vulkan::SwapChainConfig &&config);

  SwapChain(const SwapChain &) = delete;
  SwapChain &operator=(const SwapChain &) = delete;

  [[nodiscard]] vk::Format getFormat() const;
  [[nodiscard]] vk::ColorSpaceKHR getColorSpace() const;
  [[nodiscard]] const vk::Extent2D &getExtent() const;
  [[nodiscard]] LogicalDevice &getLogicalDevice();
  [[nodiscard]] Surface &getSurface();

  const vk::SwapchainKHR &operator*() const;
  vk::SwapchainKHR const *operator->() const;

  [[nodiscard]] std::string info() const override;

  [[nodiscard]] const std::vector<ImageRef> &getImages() const;
  [[nodiscard]] const std::vector<std::shared_ptr<ImageView>> &getImageViews() const;

  void swap();

 private:
  static vk::PresentModeKHR
  selectPresentMode(const std::set<vk::PresentModeKHR> &present_modes,
                    const std::vector<vk::PresentModeKHR> &surface_present_modes);

  static vk::SurfaceFormatKHR
  selectSurfaceFormat(const std::set<vk::SurfaceFormatKHR> &formats,
                      const std::vector<vk::SurfaceFormatKHR> &surfaceFormats);

  static vk::Extent2D selectExtent(const std::pair<uint32_t, uint32_t> &resolution,
                                   const vk::SurfaceCapabilitiesKHR &surfaceCapabilities);

  static vk::UniqueSwapchainKHR
  createSwapChainHandle(Surface &surface, LogicalDevice &logicalDevice,
                        const vk::SurfaceCapabilitiesKHR &surfaceCapabilities,
                        const pf::vulkan::SwapChainConfig &config,
                        vk::SurfaceFormatKHR surfaceFormat, vk::Extent2D extent,
                        vk::PresentModeKHR presentMode);

  void initImages();

  bool hasExtentChanged();

  void rebuildSwapChain(std::pair<uint32_t, uint32_t> resolution);

  vk::UniqueSwapchainKHR vkSwapChain;
  vk::Format format;
  vk::ColorSpaceKHR colorSpace;
  vk::Extent2D extent;
  std::shared_ptr<LogicalDevice> logicalDevice;
  std::shared_ptr<Surface> surface;

  std::set<vk::SurfaceFormatKHR> formats;
  std::set<vk::PresentModeKHR> presentModes;
  vk::ImageUsageFlags imageUsage;
  std::unordered_set<uint32_t> sharingQueues;
  uint32_t imageArrayLayers;
  bool clipped;
  vk::CompositeAlphaFlagBitsKHR compositeAlpha;

  std::vector<ImageRef> images;
  std::vector<std::shared_ptr<ImageView>> imageViews;
};
}// namespace pf::vulkan
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SWAPCHAIN_H
