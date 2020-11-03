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
Image::createImageView(vk::ColorSpaceKHR colorSpace, vk::ImageViewType viewType,
                       const vk::ImageSubresourceRange &subResourceRange) {
  auto imageViewConfig = ImageViewConfig{.format = format,
                                         .colorSpace = colorSpace,
                                         .viewType = viewType,
                                         .subResourceRange = subResourceRange};
  return ImageView::CreateShared(shared_from_this(), imageViewConfig);
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

  const auto requirements =
      logicalDevice->getVkLogicalDevice().getImageMemoryRequirements(*vkImage);
  auto allocateInfo = vk::MemoryAllocateInfo();
  allocateInfo.allocationSize = requirements.size;
  allocateInfo.memoryTypeIndex = findMemoryType(requirements.memoryTypeBits);
  vkMemory = logicalDevice->getVkLogicalDevice().allocateMemoryUnique(allocateInfo);
  logicalDevice->getVkLogicalDevice().bindImageMemory(*vkImage, *vkMemory, 0);
}

uint32_t ImageUnique::findMemoryType(uint32_t memoryTypeBits) {
  const auto properties =
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
  auto memoryProperties = logicalDevice->getPhysicalDevice()->getMemoryProperties();
  for (const auto &[idx, prop] : ranges::views::enumerate(memoryProperties.memoryTypes)) {
    if ((memoryTypeBits & (1u << idx)) && (prop.propertyFlags & properties) == properties) {
      return idx;
    }
  }
  throw VulkanException("Could not find fitting memory type");
}

const vk::Image &Image::operator*() const { return getVkImage(); }

vk::Image const *Image::operator->() const { return &getVkImage(); }

LogicalDevice &Image::getLogicalDevice() { return *logicalDevice; }

void Image::transitionLayout(CommandPool &cmdPool, vk::ImageLayout newLayout,
                             const vk::ImageSubresourceRange& subresourceRange) {
  auto cmdBuffer =
      cmdPool.createCommandBuffers({.level = vk::CommandBufferLevel::ePrimary, .count = 1})[0];
  auto cmdRecorder = cmdBuffer->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  auto barrier = vk::ImageMemoryBarrier();
  barrier.oldLayout = layout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = getVkImage();
  barrier.subresourceRange = subresourceRange;

  auto srcStage = vk::PipelineStageFlagBits();
  auto dstStage = vk::PipelineStageFlagBits();
  if (layout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.srcAccessMask = {};
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
    dstStage = vk::PipelineStageFlagBits::eTransfer;
  } else if (layout == vk::ImageLayout::eTransferDstOptimal
             && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    srcStage = vk::PipelineStageFlagBits::eTransfer;
    dstStage = vk::PipelineStageFlagBits::eFragmentShader;
  } else {
    throw VulkanException::fmt("Unsupported layout transition: {}",
                               magic_enum::enum_name(newLayout));
  }
  cmdRecorder.pipelineBarrier(srcStage, dstStage, {}, {}, {barrier});
  cmdRecorder.end();
  cmdBuffer->submit({.waitSemaphores = {},
                     .signalSemaphores = {},
                     .flags = {},
                     .fence = std::nullopt,
                     .wait = true});
  layout = newLayout;
}

}// namespace pf::vulkan