//
// Created by petr on 9/27/20.
//

#include "Image.h"
#include "ImageView.h"
#include "Device.h"

pf::vulkan::ImageRef::ImageRef(vk::Image img) : vkImage(img) {}

const vk::Image &pf::vulkan::ImageRef::getImage() const { return vkImage; }

std::string pf::vulkan::ImageRef::info() const { return "Vulkan image reference"; }

const vk::Image &pf::vulkan::ImageUnique::getImage() const { return vkImage.get(); }

std::string pf::vulkan::ImageUnique::info() const { return "Vulkan image unique handle"; }

pf::vulkan::ImageUnique::ImageUnique(const pf::vulkan::ImageConfig &config) {
  auto createInfo = vk::ImageCreateInfo{};
  createInfo.setImageType(config.imageType)
      .setFormat(config.format)
      .setExtent(config.extent)
      .setMipLevels(config.mipLevels)
      .setArrayLayers(config.arrayLayers)
      .setSamples(config.sampleCount)
      .setTiling(config.tiling)
      .setUsage(config.usage)
      .setInitialLayout(config.layout);
  const auto queueIndices = config.sharingQueues | ranges::to_vector;
  if (queueIndices.size() > 1) {
    createInfo.setSharingMode(vk::SharingMode::eConcurrent).setQueueFamilyIndices(queueIndices);
  } else {
    createInfo.setSharingMode(vk::SharingMode::eExclusive);
  }
  vkImage = config.logicalDevice->getVkLogicalDevice().createImageUnique(createInfo);
  imageType = config.imageType;
  format = config.format;
  extent = config.extent;
  mipLevels = config.mipLevels;
  arrayLayers = config.arrayLayers;
  sampleCount = config.sampleCount;
  tiling = config.tiling;
  layout = config.layout;
  logicalDevice = config.logicalDevice;
}

vk::ImageType pf::vulkan::ImageUnique::getImageType() const { return imageType; }

vk::Format pf::vulkan::ImageUnique::getFormat() const { return format; }

const vk::Extent3D &pf::vulkan::ImageUnique::getExtent() const { return extent; }

uint32_t pf::vulkan::ImageUnique::getMipLevels() const { return mipLevels; }

uint32_t pf::vulkan::ImageUnique::getArrayLayers() const { return arrayLayers; }

vk::SampleCountFlagBits pf::vulkan::ImageUnique::getSampleCount() const { return sampleCount; }

vk::ImageTiling pf::vulkan::ImageUnique::getTiling() const { return tiling; }

vk::ImageLayout pf::vulkan::ImageUnique::getLayout() const { return layout; }

std::shared_ptr<pf::vulkan::ImageView>
pf::vulkan::ImageUnique::createImageView(SwapChain &swapChain, vk::ColorSpaceKHR colorSpace,
                                         vk::ImageViewType viewType,
                                         const vk::ImageSubresourceRange& subResourceRange) {
  auto imageViewConfig = ImageViewConfig {
    .logicalDevice = *logicalDevice,
    .swapChain = swapChain,
    .image = *this,
    .format = format,
    .colorSpace = colorSpace,
    .viewType = viewType,
    .subResourceRange = subResourceRange
  };
  return ImageView::CreateShared(imageViewConfig);
}

const vk::Image &pf::vulkan::Image::operator*() const { return getImage(); }

vk::Image const *pf::vulkan::Image::operator->() const { return &getImage(); }
