//
// Created by petr on 9/26/20.
//

#include "SwapChain.h"
#include "../VulkanException.h"
#include <range/v3/view.hpp>

std::vector<std::shared_ptr<pf::vulkan::ImageView>>
pf::vulkan::SwapChain::createImageViews(pf::vulkan::LogicalDevice &dev) {
  auto result = std::vector<std::shared_ptr<pf::vulkan::ImageView>>();
  auto subResourceRange = vk::ImageSubresourceRange();
  subResourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setBaseMipLevel(0)
      .setLevelCount(1)
      .setBaseArrayLayer(0)
      .setLayerCount(1);
  for (const auto &img : dev.getVkLogicalDevice().getSwapchainImagesKHR(*vkSwapChain)) {
    auto imgRef = ImageRef(img);
    auto config = ImageViewConfig{.logicalDevice = dev,
                                  .swapChain = *this,
                                  .image = imgRef,
                                  .format = format,
                                  .colorSpace = colorSpace,
                                  .viewType = vk::ImageViewType::e2D,
                                  .subResourceRange = subResourceRange};
    result.emplace_back(ImageView::CreateShared(config));
  }
  return result;
}

vk::Format pf::vulkan::SwapChain::getFormat() const { return format; }

vk::ColorSpaceKHR pf::vulkan::SwapChain::getColorSpace() const { return colorSpace; }

const vk::Extent2D &pf::vulkan::SwapChain::getExtent() const { return extent; }

std::string pf::vulkan::SwapChain::info() const { return "Vulkan swapchain unique"; }

const vk::SwapchainKHR &pf::vulkan::SwapChain::operator*() const { return *vkSwapChain; }

vk::SwapchainKHR const *pf::vulkan::SwapChain::operator->() const { return &*vkSwapChain; }

pf::vulkan::SwapChain::SwapChain(const pf::vulkan::SwapChainConfig &config) {
  log(spdlog::level::info, VK_TAG, "Creating vulkan swap chain.");
  const auto surfaceFormats =
      config.device.getPhysicalDevice().getSurfaceFormatsKHR(config.surface.getSurface());
  const auto selectedSurfaceFormat = selectSurfaceFormat(config.formats, surfaceFormats);
  logFmt(spdlog::level::info, VK_TAG, "Surface format: {}, color space: {}.",
         vk::to_string(selectedSurfaceFormat.format),
         vk::to_string(selectedSurfaceFormat.colorSpace));
  const auto surfacePresentModes =
      config.device.getPhysicalDevice().getSurfacePresentModesKHR(config.surface.getSurface());
  const auto selectedPresentMode = selectPresentMode(config.presentModes, surfacePresentModes);
  logFmt(spdlog::level::info, VK_TAG, "Present mode: {}.", vk::to_string(selectedPresentMode));
  const auto surfaceCapabilities =
      config.device.getPhysicalDevice().getSurfaceCapabilitiesKHR(config.surface.getSurface());
  const auto selectedExtent = selectExtent(config.resolution, surfaceCapabilities);
  logFmt(spdlog::level::info, VK_TAG, "Extent: {}x{}.", selectedExtent.width,
         selectedExtent.height);
  vkSwapChain = createSwapChainHandle(surfaceCapabilities, config, selectedSurfaceFormat,
                                      selectedExtent, selectedPresentMode);
  format = selectedSurfaceFormat.format;
  colorSpace = selectedSurfaceFormat.colorSpace;
  extent = selectedExtent;
}
vk::PresentModeKHR pf::vulkan::SwapChain::selectPresentMode(
    const std::set<vk::PresentModeKHR> &present_modes,
    const std::vector<vk::PresentModeKHR> &surface_present_modes) {
  if (const auto selected_present_mode = findFirstCommon(present_modes, surface_present_modes);
      selected_present_mode.has_value()) {
    return *selected_present_mode;
  }
  throw VulkanException("none of the present modes is available.");
}

vk::Extent2D
pf::vulkan::SwapChain::selectExtent(const std::pair<uint32_t, uint32_t> &resolution,
                                    const vk::SurfaceCapabilitiesKHR &surfaceCapabilities) {
  if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return surfaceCapabilities.currentExtent;
  } else {
    auto extent = VkExtent2D{resolution.first, resolution.second};
    extent.width = std::clamp(extent.width, surfaceCapabilities.minImageExtent.width,
                              surfaceCapabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, surfaceCapabilities.minImageExtent.height,
                               surfaceCapabilities.maxImageExtent.height);
    return extent;
  }
}

vk::UniqueSwapchainKHR
pf::vulkan::SwapChain::createSwapChainHandle(const vk::SurfaceCapabilitiesKHR &surfaceCapabilities,
                                             const pf::vulkan::SwapChainConfig &config,
                                             vk::SurfaceFormatKHR surfaceFormat,
                                             vk::Extent2D extent, vk::PresentModeKHR presentMode) {
  using namespace ranges;
  auto imageCount = surfaceCapabilities.minImageCount + 1;
  if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
    imageCount = surfaceCapabilities.maxImageCount;
  }
  auto createInfo = vk::SwapchainCreateInfoKHR{};
  createInfo.setSurface(config.surface.getSurface())
      .setMinImageCount(imageCount)
      .setImageFormat(surfaceFormat.format)
      .setImageExtent(extent)
      .setImageColorSpace(surfaceFormat.colorSpace)
      .setImageArrayLayers(config.imageArrayLayers)
      .setImageUsage(config.imageUsage)
      .setPresentMode(presentMode)
      .setPreTransform(surfaceCapabilities.currentTransform)
      .setCompositeAlpha(config.compositeAlpha)
      .setClipped(vk::Bool32{config.clipped});
  if (config.oldSwapChain.has_value()) { createInfo.setOldSwapchain(config.oldSwapChain.value()); }
  const auto queueIndices = config.sharingQueues | to_vector;
  if (queueIndices.size() > 1) {
    createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
        .setQueueFamilyIndices(queueIndices);
  } else {
    createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
  }
  return config.logicalDevice.getVkLogicalDevice().createSwapchainKHRUnique(createInfo);
}

vk::SurfaceFormatKHR pf::vulkan::SwapChain::selectSurfaceFormat(
    const std::set<vk::SurfaceFormatKHR> &formats,
    const std::vector<vk::SurfaceFormatKHR> &surface_formats) {
  {
    if (const auto selectedFormat = findFirstCommon(formats, surface_formats);
        selectedFormat.has_value()) {
      return *selectedFormat;
    }
    throw VulkanException("none of the formats is available.");
  }
}
