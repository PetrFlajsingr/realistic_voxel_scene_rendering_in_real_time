//
// Created by petr on 9/26/20.
//

#include "SwapChain.h"
#include "../VulkanException.h"
#include "FrameBuffer.h"
#include "Image.h"
#include "ImageView.h"
#include "PhysicalDevice.h"
#include "Surface.h"
#include <range/v3/view.hpp>

namespace pf::vulkan {
using namespace ranges;

vk::Format SwapChain::getFormat() const { return format; }

vk::ColorSpaceKHR SwapChain::getColorSpace() const { return colorSpace; }

const vk::Extent2D &SwapChain::getExtent() const { return extent; }

std::string SwapChain::info() const { return "Vulkan swapchain unique"; }

const vk::SwapchainKHR &SwapChain::operator*() const { return *vkSwapChain; }

vk::SwapchainKHR const *SwapChain::operator->() const { return &*vkSwapChain; }

SwapChain::SwapChain(std::shared_ptr<Surface> surf, std::shared_ptr<LogicalDevice> device,
                     SwapChainConfig &&config)
    : logicalDevice(std::move(device)), surface(std::move(surf)), formats(config.formats),
      presentModes(config.presentModes), imageUsage(config.imageUsage),
      sharingQueues(config.sharingQueues), imageArrayLayers(config.imageArrayLayers),
      clipped(config.clipped), compositeAlpha(config.compositeAlpha) {
  log(spdlog::level::info, VK_TAG, "Creating vulkan swap chain.");
  auto &physicalDevice = logicalDevice->getPhysicalDevice();
  const auto surfaceFormats = physicalDevice->getSurfaceFormatsKHR(surface->getSurface());
  const auto selectedSurfaceFormat = selectSurfaceFormat(config.formats, surfaceFormats);
  logFmt(spdlog::level::info, VK_TAG, "Surface format: {}, color space: {}.",
         vk::to_string(selectedSurfaceFormat.format),
         vk::to_string(selectedSurfaceFormat.colorSpace));
  const auto surfacePresentModes = physicalDevice->getSurfacePresentModesKHR(surface->getSurface());
  const auto selectedPresentMode = selectPresentMode(config.presentModes, surfacePresentModes);
  logFmt(spdlog::level::info, VK_TAG, "Present mode: {}.", vk::to_string(selectedPresentMode));
  const auto surfaceCapabilities = physicalDevice->getSurfaceCapabilitiesKHR(surface->getSurface());
  const auto selectedExtent = selectExtent(config.resolution, surfaceCapabilities);
  logFmt(spdlog::level::info, VK_TAG, "Extent: {}x{}.", selectedExtent.width,
         selectedExtent.height);
  vkSwapChain = createSwapChainHandle(*surface, *logicalDevice, surfaceCapabilities, config,
                                      selectedSurfaceFormat, selectedExtent, selectedPresentMode);
  format = selectedSurfaceFormat.format;
  colorSpace = selectedSurfaceFormat.colorSpace;
  extent = selectedExtent;
}

vk::PresentModeKHR
SwapChain::selectPresentMode(const std::set<vk::PresentModeKHR> &present_modes,
                             const std::vector<vk::PresentModeKHR> &surface_present_modes) {
  if (const auto selected_present_mode = findFirstCommon(present_modes, surface_present_modes);
      selected_present_mode.has_value()) {
    return *selected_present_mode;
  }
  throw VulkanException("none of the present modes is available.");
}

vk::Extent2D SwapChain::selectExtent(const std::pair<uint32_t, uint32_t> &resolution,
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
SwapChain::createSwapChainHandle(Surface &surface, LogicalDevice &logicalDevice,
                                 const vk::SurfaceCapabilitiesKHR &surfaceCapabilities,
                                 const SwapChainConfig &config, vk::SurfaceFormatKHR surfaceFormat,
                                 vk::Extent2D extent, vk::PresentModeKHR presentMode) {
  auto imageCount = surfaceCapabilities.minImageCount + 1;
  if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
    imageCount = surfaceCapabilities.maxImageCount;
  }
  auto createInfo = vk::SwapchainCreateInfoKHR{};
  createInfo.setSurface(surface.getSurface())
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
  return logicalDevice.getVkLogicalDevice().createSwapchainKHRUnique(createInfo);
}

vk::SurfaceFormatKHR
SwapChain::selectSurfaceFormat(const std::set<vk::SurfaceFormatKHR> &formats,
                               const std::vector<vk::SurfaceFormatKHR> &surface_formats) {
  {
    if (const auto selectedFormat = findFirstCommon(formats, surface_formats);
        selectedFormat.has_value()) {
      return *selectedFormat;
    }
    throw VulkanException("none of the formats is available.");
  }
}

LogicalDevice &SwapChain::getLogicalDevice() { return *logicalDevice; }

Surface &SwapChain::getSurface() { return *surface; }

void SwapChain::initImagesAndImageViews() {
  const auto imgs = logicalDevice->getVkLogicalDevice().getSwapchainImagesKHR(*vkSwapChain);
  images =
      imgs | views::transform([&](const auto &img) {
        return ImageRef::CreateShared(logicalDevice,
                                      ImageConfig{.imageType = vk::ImageType::e2D,
                                                  .format = format,
                                                  .extent = {extent.width, extent.height, 1},
                                                  .mipLevels = 1,
                                                  .arrayLayers = 1,
                                                  .sampleCount = vk::SampleCountFlagBits::e1,
                                                  .tiling = vk::ImageTiling::eOptimal,
                                                  .usage = vk::ImageUsageFlagBits::eTransferDst,
                                                  .sharingQueues = {},
                                                  .layout = vk::ImageLayout::ePresentSrcKHR},
                                      img);
      })
      | to_vector;
  auto subresourceRange = vk::ImageSubresourceRange();
  subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount = 1;
  subresourceRange.baseArrayLayer = 0;
  subresourceRange.layerCount = 1;
  imageViews = images | views::transform([&](auto &image) {
                 return image->createImageView(colorSpace, vk::ImageViewType::e2D, subresourceRange);
               })
      | to_vector;
}

const std::vector<std::shared_ptr<ImageRef>> &SwapChain::getImages() const { return images; }

void SwapChain::swap() {
  if (hasExtentChanged()) { logicalDevice->wait(); }
}

bool SwapChain::hasExtentChanged() {
  const auto surfaceCapabilities =
      logicalDevice->getPhysicalDevice()->getSurfaceCapabilitiesKHR(**surface);
  return surfaceCapabilities.currentExtent.width != images[0]->getExtent().width
      || surfaceCapabilities.currentExtent.height != images[0]->getExtent().height;
}

void SwapChain::rebuildSwapChain(std::pair<uint32_t, uint32_t> resolution) {
  log(spdlog::level::info, VK_TAG, "Creating vulkan swap chain.");
  auto &physicalDevice = logicalDevice->getPhysicalDevice();
  const auto surfaceFormats = physicalDevice->getSurfaceFormatsKHR(surface->getSurface());
  const auto selectedSurfaceFormat = selectSurfaceFormat(formats, surfaceFormats);
  logFmt(spdlog::level::info, VK_TAG, "Surface format: {}, color space: {}.",
         vk::to_string(selectedSurfaceFormat.format),
         vk::to_string(selectedSurfaceFormat.colorSpace));
  const auto surfacePresentModes = physicalDevice->getSurfacePresentModesKHR(surface->getSurface());
  const auto selectedPresentMode = selectPresentMode(presentModes, surfacePresentModes);
  logFmt(spdlog::level::info, VK_TAG, "Present mode: {}.", vk::to_string(selectedPresentMode));
  const auto surfaceCapabilities = physicalDevice->getSurfaceCapabilitiesKHR(surface->getSurface());
  const auto selectedExtent = selectExtent(resolution, surfaceCapabilities);
  logFmt(spdlog::level::info, VK_TAG, "Extent: {}x{}.", selectedExtent.width,
         selectedExtent.height);

  auto config = SwapChainConfig{.formats = {},
                                .presentModes = {},
                                .resolution = {},
                                .imageUsage = imageUsage,
                                .sharingQueues = sharingQueues,
                                .imageArrayLayers = imageArrayLayers,
                                .clipped = clipped,
                                .oldSwapChain = *vkSwapChain,
                                .compositeAlpha = compositeAlpha};
  vkSwapChain = createSwapChainHandle(*surface, *logicalDevice, surfaceCapabilities, config,
                                      selectedSurfaceFormat, selectedExtent, selectedPresentMode);
  format = selectedSurfaceFormat.format;
  colorSpace = selectedSurfaceFormat.colorSpace;
  extent = selectedExtent;
  initImagesAndImageViews();
  initFrameBuffers();
}

const std::vector<std::shared_ptr<ImageView>> &SwapChain::getImageViews() const {
  return imageViews;
}

void SwapChain::initFrameBuffers() {
  frameBuffers = imageViews | views::transform([this](const auto &) {
                   return FrameBuffer::CreateShared(shared_from_this());
                 })
      | to_vector;
}

void SwapChain::init() {
  initImagesAndImageViews();
  initFrameBuffers();
}

}// namespace pf::vulkan