//
// Created by petr on 9/27/20.
//

#include "Image.h"
#include "ImageView.h"
#include "PhysicalDevice.h"

namespace pf::vulkan {
Image::Image(std::shared_ptr<LogicalDevice> device, ImageConfig &&config)
    : imageType(config.imageType), format(config.format), extent(config.extent),
      mipLevels(config.mipLevels), arrayLayers(config.arrayLayers), sampleCount(config.sampleCount),
      tiling(config.tiling), layout(config.layout), usage(config.usage),
      sharingQueues(std::move(config.sharingQueues)), logicalDevice(std::move(device)) {}

std::shared_ptr<ImageView>
Image::createImageView(SwapChain &swapChain, vk::ColorSpaceKHR colorSpace,
                       vk::ImageViewType viewType,
                       const vk::ImageSubresourceRange &subResourceRange) {
  auto imageViewConfig = ImageViewConfig{.logicalDevice = *logicalDevice,
                                         .swapChain = swapChain,
                                         .image = *this,
                                         .format = format,
                                         .colorSpace = colorSpace,
                                         .viewType = viewType,
                                         .subResourceRange = subResourceRange};
  return ImageView::CreateShared(imageViewConfig);
}

vk::ImageType Image::getImageType() const { return imageType; }

vk::Format Image::getFormat() const { return format; }

const vk::Extent3D &Image::getExtent() const { return extent; }

uint32_t Image::getMipLevels() const { return mipLevels; }

uint32_t Image::getArrayLayers() const { return arrayLayers; }

vk::SampleCountFlagBits Image::getSampleCount() const { return sampleCount; }

vk::ImageTiling Image::getTiling() const { return tiling; }

vk::ImageLayout Image::getLayout() const { return layout; }

ImageRef::ImageRef(std::shared_ptr<LogicalDevice> device, ImageConfig &&config, vk::Image img)
    : Image(std::move(device), std::move(config)), vkImage(img) {}

const vk::Image &ImageRef::getVkImage() const { return vkImage; }

std::string ImageRef::info() const { return "Vulkan image reference"; }

const vk::Image &ImageUnique::getVkImage() const { return vkImage.get(); }

std::string ImageUnique::info() const { return "Vulkan image unique handle"; }

ImageUnique::ImageUnique(std::shared_ptr<LogicalDevice> device, ImageConfig &&config)
    : Image(std::move(device), std::move(config)) {
  auto createInfo = vk::ImageCreateInfo{};
  createInfo.setImageType(imageType)
      .setFormat(format)
      .setExtent(extent)
      .setMipLevels(mipLevels)
      .setArrayLayers(arrayLayers)
      .setSamples(sampleCount)
      .setTiling(tiling)
      .setUsage(usage)
      .setInitialLayout(layout);
  const auto queueIndices = sharingQueues | ranges::to_vector;
  if (queueIndices.size() > 1) {
    createInfo.setSharingMode(vk::SharingMode::eConcurrent).setQueueFamilyIndices(queueIndices);
  } else {
    createInfo.setSharingMode(vk::SharingMode::eExclusive);
  }
  vkImage = logicalDevice->getVkLogicalDevice().createImageUnique(createInfo);
}

const vk::Image &Image::operator*() const { return getVkImage(); }

vk::Image const *Image::operator->() const { return &getVkImage(); }

LogicalDevice &Image::getLogicalDevice() { return *logicalDevice; }

}// namespace pf::vulkan